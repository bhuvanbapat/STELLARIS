#include "render/Renderer.h"

#include <fstream>

#include "core/Log.h"
#include "gfx/ImageTexture.h"
#include "sim/StellarModel.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>

namespace stellaris {

namespace {

// Pulsar magnetic axis: tilted from the spin axis (+Y) and rotating with the star.
// The rotation rate is slowed enormously for visibility. Shared by the beams and
// the neutron-star field lines so the two always agree.
glm::vec3 pulsarAxis(double time) {
    const float spin = static_cast<float>(time) * 2.4f;
    const float tilt = glm::radians(35.0f);
    return glm::normalize(glm::vec3(std::sin(tilt) * std::cos(spin), std::cos(tilt), std::sin(tilt) * std::sin(spin)));
}

// Orthonormal frame whose Y axis is `axis`, scaled by `scale`.
glm::mat4 axisFrame(const glm::vec3& axis, float scale) {
    const glm::vec3 ref = std::abs(axis.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    const glm::vec3 x = glm::normalize(glm::cross(ref, axis));
    const glm::vec3 z = glm::cross(x, axis);
    glm::mat4 m(1.0f);
    m[0] = glm::vec4(x * scale, 0.0f);
    m[1] = glm::vec4(axis * scale, 0.0f);
    m[2] = glm::vec4(z * scale, 0.0f);
    return m;
}

constexpr int kNoiseSize      = 128;
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
    glm::vec4 wind;         // strength, speed, dusty, unused
    glm::vec4 inner;        // streamers, giantAtmosphere, remnantData, circumstellarRing
};
static_assert(sizeof(FrameUniforms) == 4 * 64 + 14 * 16, "FrameUniforms must match std140 layout");

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
    corona_          = Shader("corona", {{GL_VERTEX_SHADER, "glow.vert"}, {GL_FRAGMENT_SHADER, "corona.frag"}});
    section_         = Shader("section", {{GL_VERTEX_SHADER, "glow.vert"}, {GL_FRAGMENT_SHADER, "section.frag"}});
    fieldline_       = Shader("fieldline", {{GL_VERTEX_SHADER, "fieldline.vert"}, {GL_FRAGMENT_SHADER, "fieldline.frag"}});
    prominence_      = Shader("prominence", {{GL_VERTEX_SHADER, "prominence.vert"}, {GL_FRAGMENT_SHADER, "prominence.frag"}});
    allShaders_ = {&noiseGen_, &sky_, &volume_, &composite_, &star_, &glow_, &disc_,
                   &particlesShader_, &beam_, &lens_, &bloomDown_, &bloomUp_, &final_, &corona_, &prominence_, &fieldline_, &section_};

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
    fieldLines_.init();
    createBlackbodyLut();
    createTargets();
    log::info("Renderer ready: noise volume {}^3, {}x{}", kNoiseSize, width_, height_);
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
    volumeTarget_ = makeTarget(static_cast<int>(width_ * quality_.volumeScale + 0.5f),
                               static_cast<int>(height_ * quality_.volumeScale + 0.5f), GL_RGBA16F, "volume");
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

void Renderer::setCloud(const CloudField& cloud) {
    const int n = cloud.resolution();
    cloudStructure_ = gl::createTexture(GL_TEXTURE_3D);
    glTextureStorage3D(cloudStructure_.id(), 1, GL_RGBA16F, n, n, n);
    glTextureSubImage3D(cloudStructure_.id(), 0, 0, 0, 0, n, n, n, GL_RGBA, GL_FLOAT, cloud.voxels().data());
    glTextureParameteri(cloudStructure_.id(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(cloudStructure_.id(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const float zero[4] = {0, 0, 0, 0};
    glTextureParameterfv(cloudStructure_.id(), GL_TEXTURE_BORDER_COLOR, zero);
    for (GLenum wrap : {GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R})
        glTextureParameteri(cloudStructure_.id(), wrap, GL_CLAMP_TO_BORDER);
    particles_.init(cloud);
    log::info("Cloud seed {}: structure {}^3 uploaded, {} particles", cloud.seed(), n, particles_.count());
}

bool Renderer::loadRemnantVolume(const std::filesystem::path& raw, int n) {
    std::ifstream in(raw, std::ios::binary | std::ios::ate);
    const std::streamoff expected = static_cast<std::streamoff>(n) * n * n * 4;
    if (!in || in.tellg() != expected) {
        log::warn("Remnant volume {} missing or incomplete; using the procedural remnant", raw.string());
        return false;
    }
    std::vector<unsigned char> data(static_cast<size_t>(expected));
    in.seekg(0);
    in.read(reinterpret_cast<char*>(data.data()), expected);
    remnantVolume_ = gl::createTexture(GL_TEXTURE_3D);
    glTextureStorage3D(remnantVolume_.id(), 1, GL_RGBA8, n, n, n);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTextureSubImage3D(remnantVolume_.id(), 0, 0, 0, 0, n, n, n, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glTextureParameteri(remnantVolume_.id(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(remnantVolume_.id(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    for (GLenum w : {GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R})
        glTextureParameteri(remnantVolume_.id(), w, GL_CLAMP_TO_BORDER);
    log::info("Remnant volume (Cas A 3D reconstruction): {}^3 RGBA8, {:.1f} MiB on GPU", n, expected / 1048576.0);
    return true;
}

bool Renderer::loadMagnetogram(const std::filesystem::path& png) {
    auto img = loadImageTexture(png, false);
    if (!img) {
        log::warn("Magnetogram {} unavailable; using the built-in active-region layout", png.string());
        return false;
    }
    magnetogram_ = std::move(img->texture);
    glTextureParameteri(magnetogram_.id(), GL_TEXTURE_WRAP_S, GL_REPEAT);  // longitude wraps
    log::info("Surface magnetogram: SDO/HMI CR 2284 ({}x{})", img->width, img->height);
    return true;
}

void Renderer::setQuality(QualityPreset preset) {
    if (preset == qualityPreset_) return;
    qualityPreset_ = preset;
    quality_ = renderQuality(preset);
    createTargets();
    log::info("Render quality: {} (volume x{:.2f}, {} steps, particles x{:.2f})", quality_.name,
              quality_.volumeScale, quality_.volumeSteps, quality_.particleFraction);
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
    // Off-axis lens shift: moves the image horizontally without changing perspective.
    // (z_view < 0 in front of the camera, hence the minus sign for a rightward shift.)
    proj_[2][0] = -lensShiftX;
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
    u.wind = {s.wind, s.windSpeed, s.windDusty, 0.0f};
    u.inner = {s.streamers, s.giantAtmosphere,
               (remnantVolume_.id() != 0 && s.pathway == Pathway::NeutronStar) ? 1.0f : 0.0f,
               s.circumstellarRing};
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
    sky_.set("uHasSkyMap", skyTexture_ != 0 ? 1 : 0);
    if (skyTexture_ != 0) glBindTextureUnit(1, skyTexture_);
    drawFullscreen();

    // --- 2. Opaque bodies ----------------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    if (s.centralPresence > 0.01f && s.centralRadius > 0.01f) {
        star_.use();
        star_.set("uHasMagnetogram", magnetogram_.id() != 0 ? 1 : 0);
        star_.set("uBand", solarBand_);
        if (magnetogram_.id() != 0) glBindTextureUnit(4, magnetogram_.id());
        star_.set("uCenter", glm::vec3(0.0f));
        star_.set("uRadius", s.centralRadius);
        star_.set("uColor", s.centralColor);
        star_.set("uIntensity", s.centralIntensity * s.centralPresence);
        star_.set("uTurbulence", s.turbulence);
        star_.set("uVeil", s.protostarVeil);
        star_.set("uLimbU", 0.6f);
        star_.set("uSeed", 0.17f);
        star_.set("uActivity", s.activity);
        star_.set("uFlare", s.flare);
        star_.set("uPixelsPerRadius", pixelsPerUnitAt(glm::vec3(0.0f), cam) * s.centralRadius);
        sphere_.draw();
    }
    if (s.remnantPresence > 0.01f) {
        star_.use();
        star_.set("uHasMagnetogram", magnetogram_.id() != 0 ? 1 : 0);
        star_.set("uBand", solarBand_);
        if (magnetogram_.id() != 0) glBindTextureUnit(4, magnetogram_.id());
        star_.set("uCenter", glm::vec3(0.0f));
        star_.set("uRadius", s.remnantRadius * (0.6f + 0.4f * s.remnantPresence));
        star_.set("uColor", s.remnantColor);
        star_.set("uIntensity", s.remnantIntensity * s.remnantPresence);
        star_.set("uTurbulence", 0.0f);
        star_.set("uVeil", 0.0f);
        star_.set("uLimbU", 0.25f);
        star_.set("uSeed", 0.61f);
        star_.set("uActivity", 0.0f);
        star_.set("uFlare", 0.0f);
        star_.set("uPixelsPerRadius", pixelsPerUnitAt(glm::vec3(0.0f), cam) * s.remnantRadius);
        sphere_.draw();
    }

    // --- 3. Volume at half resolution, then composite --------------------------------
    if (s.cloudDensity > 0.001f || s.ejectaPresence > 0.0f || s.discPresence > 0.0f || s.jetPresence > 0.0f ||
        s.streamers > 0.0f || s.giantAtmosphere > 0.0f || s.circumstellarRing > 0.0f) {
        glBindFramebuffer(GL_FRAMEBUFFER, volumeTarget_.fbo.id());
        glViewport(0, 0, volumeTarget_.width, volumeTarget_.height);
        glDisable(GL_DEPTH_TEST);
        volume_.use();
        volume_.set("uSteps", quality_.volumeSteps);
        volume_.set("uCloudExtent", CloudField::kExtent);
        volume_.set("uKeyDir", CloudField::keyLightDirection());
        glBindTextureUnit(1, hdrDepth_.id());
        glBindTextureUnit(2, cloudStructure_.id());
        if (remnantVolume_.id() != 0) glBindTextureUnit(3, remnantVolume_.id());
        drawFullscreen();

        glBindFramebuffer(GL_FRAMEBUFFER, hdrFbo_.id());
        glViewport(0, 0, width_, height_);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        composite_.use();
        glBindTextureUnit(0, volumeTarget_.color.id());
        glBindTextureUnit(1, hdrDepth_.id());
        drawFullscreen();
        glBindTextureUnit(0, noise3d_.id());
    }

    // --- 4. Transparent structures ------------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    // The disc is now a ray-marched medium (inner_medium.glsl); the layered geometric
    // disc is drawn only together with the optional tracer particles.
    if (showTracers_ && s.discPresence > 0.01f) {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        disc_.use();
        disc_.set("uLayers", kDiscLayers);
        glBindVertexArray(annulus_.vao.id());
        glDrawElementsInstanced(GL_TRIANGLES, annulus_.indexCount, GL_UNSIGNED_INT, nullptr, kDiscLayers);
    }

    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    if (showTracers_) {  // gas tracer particles: an optional inspection overlay
        particlesShader_.use();
        particlesShader_.set("uParticleFraction", quality_.particleFraction);
        particles_.draw();
    }

    if (s.pulsarBeams > 0.01f) {
        const glm::vec3 axis = pulsarAxis(s.time);
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

    // Analytic dipole field lines (explanatory overlay; normalized geometry, not MHD).
    if (showFieldLines_) {
        fieldline_.use();
        fieldline_.set("uWidthPx", 0.9f);
        if (s.atmosphere > 0.05f && s.centralRadius > 0.01f) {
            // Main-sequence star: dipole along the rotation axis, turning with the star.
            const float spin = -static_cast<float>(s.time) * 0.02f;  // = STAR_SPIN in stellar_activity.glsl
            glm::mat4 model = glm::rotate(glm::mat4(1.0f), spin, glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(s.centralRadius));
            fieldline_.set("uModel", model);
            fieldline_.set("uIntensity", 0.5f * s.atmosphere);
            fieldline_.set("uFadeRadius", 5.0f);
            fieldLines_.draw(FieldLines::Set::Star);
        }
        if (s.pulsarBeams > 0.05f) {
            // Neutron star: tilted dipole sharing the pulsar beams' magnetic axis.
            fieldline_.set("uModel", axisFrame(pulsarAxis(s.time), s.remnantRadius * (0.6f + 0.4f * s.remnantPresence)));
            fieldline_.set("uIntensity", 0.6f * s.pulsarBeams);
            fieldline_.set("uFadeRadius", 45.0f);
            fieldLines_.draw(FieldLines::Set::NeutronStar);
        }
    }

    // Main-sequence atmosphere: prominences, then chromosphere + corona. Depth-tested
    // so the stellar sphere hides whatever lies behind its disc.
    if (s.atmosphere > 0.01f && s.centralRadius > 0.01f) {
        if (s.activity > 0.01f) {
            constexpr int kSegments = 48;
            prominence_.use();
            prominence_.set("uRadius", s.centralRadius);
            prominence_.set("uSegments", kSegments);
            prominence_.set("uProminenceColor", solarBand_ == 1 ? glm::vec3(1.0f, 0.42f, 0.10f)
                                              : solarBand_ == 2 ? glm::vec3(0.55f, 0.40f, 0.15f) : glm::vec3(1.0f, 0.34f, 0.40f));
            prominence_.set("uIntensity", s.activity * s.atmosphere * s.centralIntensity * 0.9f);
            glBindVertexArray(emptyVao_.id());
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 2 * (kSegments + 1), 3);
        }
        corona_.use();
        corona_.set("uHasMagnetogram", magnetogram_.id() != 0 ? 1 : 0);
        corona_.set("uBand", solarBand_);
        if (magnetogram_.id() != 0) glBindTextureUnit(4, magnetogram_.id());
        corona_.set("uCenter", glm::vec3(0.0f));
        corona_.set("uSize", s.centralRadius * 4.5f);
        corona_.set("uRadius", s.centralRadius);
        corona_.set("uColor", s.centralColor);
        corona_.set("uIntensity", s.centralIntensity * s.atmosphere * 0.32f);
        corona_.set("uActivity", s.activity);
        corona_.set("uFlare", s.flare);
        glBindVertexArray(emptyVao_.id());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // Halos (drawn without depth test: they are optical glare around the source).
    glDisable(GL_DEPTH_TEST);
    if (s.centralPresence > 0.01f) {
        const glm::vec3 haloColor = glm::mix(s.centralColor, glm::vec3(1.0f, 0.4f, 0.15f), s.protostarVeil);
        const float haloScale = 5.0f - 3.2f * s.giant;  // giants: thin halo, not a flood of light
        drawGlow(glm::vec3(0.0f), s.centralRadius * haloScale, 1.0f / haloScale, 2.8f, haloColor,
                 s.centralIntensity * s.centralPresence * 0.16f * (1.0f - 0.75f * s.atmosphere), false);
    }
    if (s.remnantPresence > 0.01f) {
        drawGlow(glm::vec3(0.0f), s.remnantRadius * 30.0f, 1.0f / 30.0f, 0.35f, s.remnantColor,
                 s.remnantIntensity * s.remnantPresence * 0.25f, false);
    }
    if (s.supernovaFlash > 0.001f) {
        drawGlow(glm::vec3(0.0f), 34.0f, 0.03f, 0.9f, glm::vec3(0.85f, 0.92f, 1.0f), s.supernovaFlash * 35.0f, true);
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

    // Scientific cross-section through the core, facing the camera (overlay).
    if (showSection_ && s.cloudDensity > 0.01f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        section_.use();
        section_.set("uCenter", glm::vec3(0.0f));
        section_.set("uSize", CloudField::kExtent);
        section_.set("uRangeMax", CloudField::kSectionRangeMax);
        section_.set("uCloudExtent", CloudField::kExtent);
        section_.set("uKeyDir", CloudField::keyLightDirection());
        glBindTextureUnit(0, noise3d_.id());
        glBindTextureUnit(2, cloudStructure_.id());
        glBindVertexArray(emptyVao_.id());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisable(GL_BLEND);
    }

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

float Renderer::pixelsPerUnitAt(const glm::vec3& world, const CameraPose& cam) const {
    return proj_[1][1] * 0.5f * static_cast<float>(height_) / std::max(glm::length(world - cam.position), 1e-3f);
}

std::optional<glm::vec2> Renderer::projectToScreen(const glm::vec3& world) const {
    const glm::vec4 clip = proj_ * view_ * glm::vec4(world, 1.0f);
    if (clip.w <= 1e-4f) return std::nullopt;
    const glm::vec3 ndc = glm::vec3(clip) / clip.w;
    return glm::vec2((ndc.x * 0.5f + 0.5f) * width_, (1.0f - (ndc.y * 0.5f + 0.5f)) * height_);
}

} // namespace stellaris
