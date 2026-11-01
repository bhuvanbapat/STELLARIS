#include "render/Renderer.h"

#include "core/Log.h"
#include "sim/StellarModel.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>

namespace stellaris {

namespace {

constexpr int kNoiseSize      = 128;
constexpr int kVolumeSteps    = 72;
constexpr int kBloomMips      = 6;
constexpr int kDiscLayers     = 5;
constexpr int kBlackbodyLutN  = 256;

// Mirrors `uniform Frame` in shaders/common.glsl (std140: only mat4/vec4 members).
struct FrameUniforms {
    glm::mat4 view, proj, viewProj, invViewProj;
    glm::vec4 camPos;       // xyz, w = timeline time
    glm::vec4 viewport;     // w, h, 1/w, 1/h
    glm::vec4 cloud;        // cloudDensity, collapse, coreGlow, cavity
    glm::vec4 cloud2;       // dispersal, infallClock, rotation, discScale
    glm::vec4 central;      // presence, radius, intensity, turbulence
    glm::vec4 centralColor; // rgb, veil
    glm::vec4 disc;         // presence, outer, inner, jetPresence
    glm::vec4 misc;         // jetLength, giant, fateProgress, exposure
    glm::vec4 ejecta;       // presence, clock, kind, glow
    glm::vec4 ejecta2;      // strength, flash, remnantPresence, remnantRadius
    glm::vec4 remnantColor; // rgb, intensity
    glm::vec4 blackHole;    // presence, shadowR, einsteinR, pulsarBeams
};
static_assert(sizeof(FrameUniforms) == 4 * 64 + 12 * 16, "FrameUniforms must match std140 layout");

gl::Texture makeTexture2D(int w, int h, GLenum format, GLenum filter) {
    gl::Texture t = gl::createTexture(GL_TEXTURE_2D);
    glTextureStorage2D(t.id(), 1, format, w, h);
    glTextureParameteri(t.id(), GL_TEXTURE_MIN_FILTER, filter);
    glTextureParameteri(t.id(), GL_TEXTURE_MAG_FILTER, filter);
    glTextureParameteri(t.id(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(t.id(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return t;
}

bool framebufferComplete(GLuint fbo, const char* label) {
    const GLenum status = glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        log::error("Framebuffer '{}' incomplete (0x{:x})", label, status);
        return false;
    }
    return true;
}

} // namespace

bool Renderer::init(const std::filesystem::path& shaderDir, int width, int height) {
    shaderDir_ = shaderDir;
    width_  = std::max(width, 1);
    height_ = std::max(height, 1);

    noiseGen_        = Shader("noise3d", {{GL_COMPUTE_SHADER, "noise3d.comp"}});
    sky_             = Shader("sky", {{GL_VERTEX_SHADER, "fullscreen.vert"}, {GL_FRAGMENT_SHADER, "sky.frag"}});
    volume_          = Shader("volume", {{GL_VERTEX_SHADER, "fullscreen.vert"}, {GL_FRAGMENT_SHADER, "volume.frag"}});
    composite_       = Shader("composite", {{GL_VERTEX_SHADER, "fullscreen.vert"}, {GL_FRAGMENT_SHADER, "composite.frag"}});
    star_            = Shader("star", {{GL_VERTEX_SHADER, "star.vert"}, {GL_FRAGMENT_SHADER, "star.frag"}});
    glow_            = Shader("glow", {{GL_VERTEX_SHADER, "glow.vert"}, {GL_FRAGMENT_SHADER, "glow.frag"}});
    disc_            = Shader("disc", {{GL_VERTEX_SHADER, "disc.vert"}, {GL_FRAGMENT_SHADER, "disc.frag"}});
    particlesShader_ = Shader("particles", {{GL_VERTEX_SHADER, "particles.vert"}, {GL_FRAGMENT_SHADER, "particles.frag"}});
    beam_            = Shader("beam", {{GL_VERTEX_SHADER, "beam.vert"}, {GL_FRAGMENT_SHADER, "beam.frag"}});
    lens_            = Shader("lens", {{GL_VERTEX_SHADER, "fullscreen.vert"}, {GL_FRAGMENT_SHADER, "lens.frag"}});
    bloomDown_       = Shader("bloom_down", {{GL_VERTEX_SHADER, "fullscreen.vert"}, {GL_FRAGMENT_SHADER, "bloom_down.frag"}});
    bloomUp_         = Shader("bloom_up", {{GL_VERTEX_SHADER, "fullscreen.vert"}, {GL_FRAGMENT_SHADER, "bloom_up.frag"}});
    final_           = Shader("final", {{GL_VERTEX_SHADER, "fullscreen.vert"}, {GL_FRAGMENT_SHADER, "final.frag"}});
    allShaders_ = {&noiseGen_, &sky_, &volume_, &composite_, &star_, &glow_, &disc_,
                   &particlesShader_, &beam_, &lens_, &bloomDown_, &bloomUp_, &final_};

    bool ok = true;
    for (Shader* s : allShaders_) ok = s->load(shaderDir_) && ok;
    if (!ok) {
        log::error("One or more shaders failed to compile; see messages above.");
        return false;
    }

    sphere_  = makeSphere(128, 64);
    annulus_ = makeAnnulusGrid(96, 720);
    cone_    = makeCone(64, 16);
    emptyVao_ = gl::createVertexArray();
    frameUbo_ = gl::createBuffer();
    glNamedBufferStorage(frameUbo_.id(), sizeof(FrameUniforms), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glGenQueries(static_cast<GLsizei>(timeQueries_.size()), timeQueries_.data());

    createNoiseVolume();
    createBlackbodyLut();
    particles_.init();
    createTargets();
    log::info("Renderer ready: {} particles, noise volume {}^3, {}x{}", particles_.count(), kNoiseSize, width_, height_);
    return true;
}

Renderer::~Renderer() {
    if (timeQueries_[0] != 0) glDeleteQueries(static_cast<GLsizei>(timeQueries_.size()), timeQueries_.data());
}

bool Renderer::reloadShaders() {
    bool ok = true;
    for (Shader* s : allShaders_) {
        if (s == &noiseGen_) continue;  // the noise volume is generated once
        ok = s->load(shaderDir_) && ok;
    }
    if (ok) log::info("Shaders reloaded.");
    else log::warn("Shader reload had errors; previous programs kept where needed.");
    return ok;
}

void Renderer::resize(int width, int height) {
    width  = std::max(width, 1);
    height = std::max(height, 1);
    if (width == width_ && height == height_) return;
    width_ = width;
    height_ = height;
    createTargets();
}

void Renderer::createTargets() {
    hdrColor_ = makeTexture2D(width_, height_, GL_RGBA16F, GL_LINEAR);
    hdrDepth_ = makeTexture2D(width_, height_, GL_DEPTH_COMPONENT32F, GL_NEAREST);
    hdrFbo_   = gl::createFramebuffer();
    glNamedFramebufferTexture(hdrFbo_.id(), GL_COLOR_ATTACHMENT0, hdrColor_.id(), 0);
    glNamedFramebufferTexture(hdrFbo_.id(), GL_DEPTH_ATTACHMENT, hdrDepth_.id(), 0);
    framebufferComplete(hdrFbo_.id(), "hdr");

    auto makeTarget = [](int w, int h, GLenum format, const char* label) {
        Target t;
        t.width = std::max(w, 1);
        t.height = std::max(h, 1);
        t.color = makeTexture2D(t.width, t.height, format, GL_LINEAR);
        t.fbo = gl::createFramebuffer();
        glNamedFramebufferTexture(t.fbo.id(), GL_COLOR_ATTACHMENT0, t.color.id(), 0);
        framebufferComplete(t.fbo.id(), label);
        return t;
    };
    volumeTarget_ = makeTarget(width_ / 2, height_ / 2, GL_RGBA16F, "volume");
    lensTarget_   = makeTarget(width_, height_, GL_RGBA16F, "lens");
    bloomMips_.clear();
    int w = width_, h = height_;
    for (int i = 0; i < kBloomMips; ++i) {
        w = std::max(w / 2, 1);
        h = std::max(h / 2, 1);
        bloomMips_.push_back(makeTarget(w, h, GL_R11F_G11F_B10F, "bloom"));
    }
}

void Renderer::createNoiseVolume() {
    noise3d_ = gl::createTexture(GL_TEXTURE_3D);
    glTextureStorage3D(noise3d_.id(), 1, GL_RGBA16F, kNoiseSize, kNoiseSize, kNoiseSize);
    glTextureParameteri(noise3d_.id(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(noise3d_.id(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    for (GLenum wrap : {GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R})
        glTextureParameteri(noise3d_.id(), wrap, GL_REPEAT);

    noiseGen_.use();
    noiseGen_.set("uSize", kNoiseSize);
    glBindImageTexture(0, noise3d_.id(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glDispatchCompute(kNoiseSize / 8, kNoiseSize / 8, kNoiseSize / 8);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Renderer::createBlackbodyLut() {
    // Same function the scientific model uses, sampled over log10(T) in [3, 4.7].
    std::vector<float> data(kBlackbodyLutN * 3);
    for (int i = 0; i < kBlackbodyLutN; ++i) {
        const double logT = 3.0 + 1.7 * static_cast<double>(i) / (kBlackbodyLutN - 1);
        const glm::vec3 c = blackbodyColor(std::pow(10.0, logT));
        data[i * 3 + 0] = c.r; data[i * 3 + 1] = c.g; data[i * 3 + 2] = c.b;
    }
    blackbodyLut_ = gl::createTexture(GL_TEXTURE_1D);
    glTextureStorage1D(blackbodyLut_.id(), 1, GL_RGB16F, kBlackbodyLutN);
    glTextureSubImage1D(blackbodyLut_.id(), 0, 0, kBlackbodyLutN, GL_RGB, GL_FLOAT, data.data());
    glTextureParameteri(blackbodyLut_.id(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(blackbodyLut_.id(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(blackbodyLut_.id(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
}

void Renderer::uploadFrameUniforms(const SceneState& s, const CameraPose& cam, float lensShiftX) {
    const float aspect = static_cast<float>(width_) / static_cast<float>(height_);
    view_ = glm::lookAt(cam.position, cam.target, glm::vec3(0, 1, 0));
    proj_ = glm::perspective(glm::radians(cam.fovYDegrees), aspect, 0.05f, 2000.0f);
    proj_[2][0] = lensShiftX;  // off-axis lens shift (moves the image, keeps perspective)
    camPos_ = cam.position;

    FrameUniforms u{};
    u.view = view_;
    u.proj = proj_;
    u.viewProj = proj_ * view_;
    u.invViewProj = glm::inverse(u.viewProj);
    u.camPos = glm::vec4(cam.position, static_cast<float>(s.time));
    u.viewport = glm::vec4(width_, height_, 1.0f / width_, 1.0f / height_);
    u.cloud = {s.cloudDensity, s.collapse, s.coreGlow, s.cavity};
    u.cloud2 = {s.dispersal, s.infallClock, s.rotation, s.discScale};
    u.central = {s.centralPresence, s.centralRadius, s.centralIntensity, s.turbulence};
    u.centralColor = glm::vec4(s.centralColor, s.protostarVeil);
    u.disc = {s.discPresence, s.discOuterRadius, s.discInnerRadius, s.jetPresence};
    u.misc = {s.jetLength, s.giant, s.fateProgress, s.exposure};
    u.ejecta = {s.ejectaPresence, s.ejectaClock, static_cast<float>(s.ejectaKind), s.ejectaGlow};
    u.ejecta2 = {s.ejectaStrength, s.supernovaFlash, s.remnantPresence, s.remnantRadius};
    u.remnantColor = glm::vec4(s.remnantColor, s.remnantIntensity);
    u.blackHole = {s.blackHole, s.bhShadowRadius, s.bhEinsteinRadius, s.pulsarBeams};
    glNamedBufferSubData(frameUbo_.id(), 0, sizeof(u), &u);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, frameUbo_.id());
}

void Renderer::drawFullscreen() {
    glBindVertexArray(emptyVao_.id());
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer::drawGlow(const glm::vec3& center, float size, float innerFrac, float falloff,
                        const glm::vec3& color, float intensity, bool rays) {
    if (intensity <= 0.0f || size <= 0.0f) return;
    glow_.use();
    glow_.set("uCenter", center);
    glow_.set("uSize", size);
    glow_.set("uInnerFrac", innerFrac);
    glow_.set("uFalloff", falloff);
    glow_.set("uColor", color);
    glow_.set("uIntensity", intensity);
    glow_.set("uRays", rays ? 1 : 0);
    glBindVertexArray(emptyVao_.id());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Renderer::render(const SceneState& s, const CameraPose& cam, float lensShiftX) {
    // GPU timing: read the oldest query (issued kQueries frames ago), start a new one.
    const int nq = static_cast<int>(timeQueries_.size());
    if (queryFrame_ >= nq) {
        GLuint64 ns = 0;
        glGetQueryObjectui64v(timeQueries_[queryFrame_ % nq], GL_QUERY_RESULT, &ns);
        const float ms = static_cast<float>(ns) * 1e-6f;
        gpuMs_ = gpuMs_ == 0.0f ? ms : gpuMs_ * 0.9f + ms * 0.1f;
    }
    glBeginQuery(GL_TIME_ELAPSED, timeQueries_[queryFrame_ % nq]);

    uploadFrameUniforms(s, cam, lensShiftX);
    glBindTextureUnit(0, noise3d_.id());
    glBindTextureUnit(7, blackbodyLut_.id());

    // --- 1. Sky ------------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFbo_.id());
    glViewport(0, 0, width_, height_);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    sky_.use();
    drawFullscreen();

    // --- 2. Opaque bodies ----------------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    if (s.centralPresence > 0.01f && s.centralRadius > 0.01f) {
        star_.use();
        star_.set("uCenter", glm::vec3(0.0f));
        star_.set("uRadius", s.centralRadius);
        star_.set("uColor", s.centralColor);
        star_.set("uIntensity", s.centralIntensity * s.centralPresence);
        star_.set("uTurbulence", s.turbulence);
        star_.set("uVeil", s.protostarVeil);
        star_.set("uLimbU", 0.6f);
        star_.set("uSeed", 0.17f);
        sphere_.draw();
    }
    if (s.remnantPresence > 0.01f) {
        star_.use();
        star_.set("uCenter", glm::vec3(0.0f));
        star_.set("uRadius", s.remnantRadius * (0.6f + 0.4f * s.remnantPresence));
        star_.set("uColor", s.remnantColor);
        star_.set("uIntensity", s.remnantIntensity * s.remnantPresence);
        star_.set("uTurbulence", 0.0f);
        star_.set("uVeil", 0.0f);
        star_.set("uLimbU", 0.25f);
        star_.set("uSeed", 0.61f);
        sphere_.draw();
    }

    // --- 3. Volume at half resolution, then composite --------------------------------
    if (s.cloudDensity > 0.001f || s.ejectaPresence > 0.0f) {
        glBindFramebuffer(GL_FRAMEBUFFER, volumeTarget_.fbo.id());
        glViewport(0, 0, volumeTarget_.width, volumeTarget_.height);
        glDisable(GL_DEPTH_TEST);
        volume_.use();
        volume_.set("uSteps", kVolumeSteps);
        glBindTextureUnit(1, hdrDepth_.id());
        drawFullscreen();

        glBindFramebuffer(GL_FRAMEBUFFER, hdrFbo_.id());
        glViewport(0, 0, width_, height_);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        composite_.use();
        glBindTextureUnit(0, volumeTarget_.color.id());
        drawFullscreen();
        glBindTextureUnit(0, noise3d_.id());
    }

    // --- 4. Transparent structures ------------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    if (s.discPresence > 0.01f) {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        disc_.use();
        disc_.set("uLayers", kDiscLayers);
        glBindVertexArray(annulus_.vao.id());
        glDrawElementsInstanced(GL_TRIANGLES, annulus_.indexCount, GL_UNSIGNED_INT, nullptr, kDiscLayers);
    }

    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    particlesShader_.use();
    particles_.draw();

    if (s.pulsarBeams > 0.01f) {
        // Magnetic axis tilted from the spin axis (+Y), rotating with the star.
        // The rotation rate is slowed enormously for visibility.
        const float spin = static_cast<float>(s.time) * 2.4f;
        const float tilt = glm::radians(35.0f);
        const glm::vec3 axis = glm::normalize(glm::vec3(std::sin(tilt) * std::cos(spin), std::cos(tilt),
                                                        std::sin(tilt) * std::sin(spin)));
        beam_.use();
        beam_.set("uIntensity", s.pulsarBeams * 0.45f);
        for (float sign : {1.0f, -1.0f}) {
            const glm::vec3 a = axis * sign;
            const glm::vec3 ref = std::abs(a.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
            const glm::vec3 x = glm::normalize(glm::cross(ref, a));
            const glm::vec3 z = glm::cross(x, a);
            glm::mat4 model(1.0f);
            model[0] = glm::vec4(x * 0.75f, 0.0f);
            model[1] = glm::vec4(a * 18.0f, 0.0f);
            model[2] = glm::vec4(z * 0.75f, 0.0f);
            beam_.set("uModel", model);
            beam_.set("uAxis", a);
            cone_.draw();
        }
    }

    // Halos (drawn without depth test: they are optical glare around the source).
    glDisable(GL_DEPTH_TEST);
    if (s.centralPresence > 0.01f) {
        const glm::vec3 haloColor = glm::mix(s.centralColor, glm::vec3(1.0f, 0.4f, 0.15f), s.protostarVeil);
        const float haloScale = 5.0f - 3.2f * s.giant;  // giants: thin halo, not a flood of light
        drawGlow(glm::vec3(0.0f), s.centralRadius * haloScale, 1.0f / haloScale, 2.8f, haloColor,
                 s.centralIntensity * s.centralPresence * 0.16f, false);
    }
    if (s.remnantPresence > 0.01f) {
        drawGlow(glm::vec3(0.0f), s.remnantRadius * 30.0f, 1.0f / 30.0f, 0.35f, s.remnantColor,
                 s.remnantIntensity * s.remnantPresence * 0.25f, false);
    }
    if (s.supernovaFlash > 0.001f) {
        drawGlow(glm::vec3(0.0f), 34.0f, 0.03f, 0.9f, glm::vec3(0.85f, 0.92f, 1.0f), s.supernovaFlash * 60.0f, true);
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // --- 5. Lensing ------------------------------------------------------------------
    GLuint sceneTex = hdrColor_.id();
    if (s.blackHole > 0.001f) {
        const auto center = projectToScreen(glm::vec3(0.0f));
        const float dist = glm::length(cam.position);
        const float pxPerUnit = proj_[1][1] * 0.5f * static_cast<float>(height_) / std::max(dist, 1e-3f);
        if (center) {
            glBindFramebuffer(GL_FRAMEBUFFER, lensTarget_.fbo.id());
            glViewport(0, 0, width_, height_);
            lens_.use();
            lens_.set("uCenterUV", glm::vec2(center->x / width_, 1.0f - center->y / height_));
            lens_.set("uShadowPx", s.bhShadowRadius * pxPerUnit);
            lens_.set("uEinsteinPx", s.bhEinsteinRadius * pxPerUnit);
            lens_.set("uPresence", s.blackHole);
            glBindTextureUnit(0, hdrColor_.id());
            drawFullscreen();
            sceneTex = lensTarget_.color.id();
        }
    }

    // --- 6. Bloom and final composite ----------------------------------------------------
    bloom(sceneTex);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width_, height_);
    final_.use();
    final_.set("uBloomStrength", 0.045f + 0.25f * s.supernovaFlash);
    glBindTextureUnit(0, sceneTex);
    glBindTextureUnit(1, bloomMips_[0].color.id());
    drawFullscreen();

    glEndQuery(GL_TIME_ELAPSED);
    ++queryFrame_;
}

void Renderer::bloom(GLuint source) {
    glDisable(GL_BLEND);
    bloomDown_.use();
    GLuint src = source;
    int srcW = width_, srcH = height_;
    for (int i = 0; i < kBloomMips; ++i) {
        const Target& dst = bloomMips_[i];
        glBindFramebuffer(GL_FRAMEBUFFER, dst.fbo.id());
        glViewport(0, 0, dst.width, dst.height);
        bloomDown_.set("uSrcTexel", glm::vec2(1.0f / srcW, 1.0f / srcH));
        bloomDown_.set("uFirst", i == 0 ? 1 : 0);
        glBindTextureUnit(0, src);
        drawFullscreen();
        src = dst.color.id();
        srcW = dst.width;
        srcH = dst.height;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    bloomUp_.use();
    for (int i = kBloomMips - 1; i > 0; --i) {
        const Target& from = bloomMips_[i];
        const Target& to = bloomMips_[i - 1];
        glBindFramebuffer(GL_FRAMEBUFFER, to.fbo.id());
        glViewport(0, 0, to.width, to.height);
        bloomUp_.set("uSrcTexel", glm::vec2(1.0f / from.width, 1.0f / from.height));
        glBindTextureUnit(0, from.color.id());
        drawFullscreen();
    }
    glDisable(GL_BLEND);
}

std::optional<glm::vec2> Renderer::projectToScreen(const glm::vec3& world) const {
    const glm::vec4 clip = proj_ * view_ * glm::vec4(world, 1.0f);
    if (clip.w <= 1e-4f) return std::nullopt;
    const glm::vec3 ndc = glm::vec3(clip) / clip.w;
    return glm::vec2((ndc.x * 0.5f + 0.5f) * width_, (1.0f - (ndc.y * 0.5f + 0.5f)) * height_);
}

} // namespace stellaris
