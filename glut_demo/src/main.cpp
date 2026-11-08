// STELLARIS - freeGLUT / OpenGL demonstration
// ===========================================
// A self-contained program built only on GLUT (freeGLUT), OpenGL 1.x/2.x
// fixed-function rendering and GLU. It retells the STELLARIS life cycle in six
// short phases and deliberately exercises the classic GLUT/OpenGL toolkit:
//
//   GLUT : window + double-buffered RGBA/depth/multisample context, display,
//          reshape, keyboard, special-key, mouse, motion, mouse-wheel and
//          timer callbacks, pop-up menus (right click), bitmap text, built-in
//          solid/wire spheres, full-screen toggle.
//   GL   : matrix stack transforms, perspective projection (gluPerspective,
//          gluLookAt), Phong-style fixed-function lighting with a point light
//          placed in the star, material emission, display lists, additive
//          blending, alpha blending, point sprites via GL_POINT_SMOOTH,
//          depth-buffer control, exponential fog for depth cueing.
//
// The animation is a pure function of a time value t (deterministic), so
// pausing, stepping phases and resetting always give the same picture.
// This is a simplified illustration; see the main application for the full
// visualization and SCIENCE_NOTES.md for the scientific model.

#include <GL/freeglut.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace {

constexpr float kPi = 3.14159265f;
constexpr float kPhaseSeconds = 10.0f;
constexpr int   kPhaseCount = 6;
const char* const kPhaseNames[kPhaseCount] = {
    "1. Molecular cloud", "2. Gravitational collapse", "3. Protostar",
    "4. Accretion disc and bipolar jets", "5. Main-sequence star", "6. Red giant -> planetary nebula -> white dwarf"};
const char* const kPhaseNotes[kPhaseCount] = {
    "Cold gas and dust; a dense core will become unstable.",
    "Inner gas falls in first (shorter free-fall time).",
    "Accretion and contraction heat a central protostar.",
    "Angular momentum forms a rotating disc; jets leave along the axis.",
    "Hydrogen fusion in the core: a stable star. The planet (illustrative) is lit by it.",
    "One illustrative low-mass pathway (no supernova)."};

// ---------------------------------------------------------------- state ----
struct DemoState {
    float time = 0.0f;       // animation time (s); all visuals derive from it
    float speed = 1.0f;
    bool  paused = false;
    bool  wireframe = false;
    bool  showHelp = true;
    bool  fog = true;
    float yaw = 25.0f, pitch = 18.0f, distance = 42.0f;
    int   dragX = 0, dragY = 0;
    bool  dragging = false;
    int   width = 1280, height = 720;
    GLuint cloudList = 0;     // display list holding the static cloud geometry
    std::string capturePath;  // --capture: render one frame and save a PNG
} g;

struct Particle { float x, y, z, seed; };
std::vector<Particle> g_cloud;

// Small deterministic RNG (xorshift) so every run is identical.
uint32_t g_rng = 2463534242u;
float rnd() { g_rng ^= g_rng << 13; g_rng ^= g_rng >> 17; g_rng ^= g_rng << 5; return (g_rng & 0xFFFFFF) / 16777216.0f; }

float smooth01(float e0, float e1, float x) {
    const float t = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
int   currentPhase() { return std::min(static_cast<int>(g.time / kPhaseSeconds), kPhaseCount - 1); }
float phaseProgress() { return std::clamp(g.time / kPhaseSeconds - currentPhase(), 0.0f, 1.0f); }

// ------------------------------------------------------------- geometry ----
void buildCloud() {
    // Clumpy, flattened cloud: particles placed around a few blob centers.
    const float centers[5][3] = {{0, 0, 0}, {-9, 3, 4}, {10, -2, -5}, {3, 5, 9}, {-4, -4, -8}};
    g_cloud.clear();
    for (int i = 0; i < 7000; ++i) {
        const float* c = centers[i % 5];
        const float r = (i % 5 == 0 ? 7.0f : 5.0f) * std::cbrt(rnd());
        const float u = rnd() * 2.0f - 1.0f, a = rnd() * 2.0f * kPi;
        const float s = std::sqrt(1.0f - u * u);
        g_cloud.push_back({c[0] + r * s * std::cos(a), c[1] + r * u * 0.6f, c[2] + r * s * std::sin(a), rnd()});
    }
    // The undisturbed cloud never changes, so compile it once into a display list.
    g.cloudList = glGenLists(1);
    glNewList(g.cloudList, GL_COMPILE);
    glBegin(GL_POINTS);
    for (const Particle& p : g_cloud) {
        glColor4f(0.35f + 0.25f * p.seed, 0.40f, 0.85f, 0.35f);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();
    glEndList();
}

void drawText(float x, float y, const char* s, void* font = GLUT_BITMAP_HELVETICA_12) {
    glRasterPos2f(x, y);
    glutBitmapString(font, reinterpret_cast<const unsigned char*>(s));
}

// Additive camera-facing glow made of concentric translucent discs.
void drawGlow(float radius, float r, float gcol, float b, float strength) {
    GLfloat m[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, m);
    // Remove rotation from the modelview so the disc faces the camera (billboard).
    glPushMatrix();
    const float sx = std::sqrt(m[0] * m[0] + m[1] * m[1] + m[2] * m[2]);
    GLfloat bb[16] = {sx, 0, 0, 0, 0, sx, 0, 0, 0, 0, sx, 0, m[12], m[13], m[14], 1};
    glLoadMatrixf(bb);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for (int k = 1; k <= 12; ++k) {
        const float rr = radius * (1.0f + k * 0.35f);
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(r, gcol, b, strength / (k * 1.5f));
        glVertex3f(0, 0, 0);
        glColor4f(r, gcol, b, 0.0f);
        for (int i = 0; i <= 48; ++i) {
            const float a = i * 2.0f * kPi / 48.0f;
            glVertex3f(rr * std::cos(a), rr * std::sin(a), 0);
        }
        glEnd();
    }
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPopMatrix();
}

// Lit sphere with the star's own light placed at its center.
void drawStar(float radius, float r, float gc, float b, float emission) {
    const GLfloat em[] = {r * emission, gc * emission, b * emission, 1.0f};
    const GLfloat diff[] = {r, gc, b, 1.0f};
    glEnable(GL_LIGHTING);
    glMaterialfv(GL_FRONT, GL_EMISSION, em);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
    if (g.wireframe) glutWireSphere(radius, 32, 20);
    else glutSolidSphere(radius, 64, 40);
    const GLfloat none[] = {0, 0, 0, 1};
    glMaterialfv(GL_FRONT, GL_EMISSION, none);
    glDisable(GL_LIGHTING);
    drawGlow(radius, r, gc, b, 0.22f);
}

// Accretion disc: rings rotate with Keplerian angular velocity (omega ~ r^-1.5),
// colored from hot (inner) to cool (outer).
void drawDisc(float presence, float outer) {
    if (presence <= 0.0f) return;
    const float inner = 1.6f;
    const int rings = 28, segs = 96;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for (int i = 0; i < rings; ++i) {
        const float r0 = inner + (outer - inner) * i / rings;
        const float r1 = inner + (outer - inner) * (i + 1) / rings;
        const float omega = 4.0f * std::pow(r0, -1.5f);
        const float rot = omega * g.time;
        const float heat = 1.0f - static_cast<float>(i) / rings;
        glBegin(g.wireframe ? GL_LINES : GL_TRIANGLE_STRIP);
        for (int s = 0; s <= segs; ++s) {
            const float a = s * 2.0f * kPi / segs;
            // Spiral streaks: brightness varies with angle relative to the ring rotation.
            const float streak = 0.55f + 0.45f * std::sin(3.0f * (a - rot) + i * 0.7f);
            glColor4f(1.0f, 0.45f + 0.5f * heat, 0.15f + 0.6f * heat * heat, presence * 0.22f * streak);
            glVertex3f(r0 * std::cos(a), 0.0f, r0 * std::sin(a));
            glVertex3f(r1 * std::cos(a), 0.0f, r1 * std::sin(a));
        }
        glEnd();
    }
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Bipolar jets: points streaming outward along +/-Y with moving knots.
void drawJets(float presence) {
    if (presence <= 0.0f) return;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glPointSize(2.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 1600; ++i) {
        const float seed = (i * 0.618034f) - std::floor(i * 0.618034f);
        const float phase = std::fmod(seed + g.time * 0.15f, 1.0f);
        const float y = 1.8f + phase * 18.0f;
        const float side = (i & 1) ? 1.0f : -1.0f;
        const float spread = 0.05f + 0.04f * y;
        // Independent hash for the angle (a value correlated with `seed` makes a corkscrew).
        const float a = ((static_cast<uint32_t>(i) * 2654435761u) >> 8 & 0xFFFF) / 65535.0f * 2.0f * kPi;
        const float knot = 0.4f + 0.6f * std::pow(0.5f + 0.5f * std::sin((y - 2.7f * g.time) * 0.8f), 3.0f);
        glColor4f(0.5f, 0.75f, 1.0f, presence * knot * (1.0f - phase));
        // Fill the cone's cross-section (area-uniform), not just its surface.
        const float rr = spread * std::sqrt(((static_cast<uint32_t>(i) * 2246822519u + 374761393u) >> 8 & 0xFFFF) / 65535.0f);
        glVertex3f(rr * std::cos(a), side * y, rr * std::sin(a));
    }
    glEnd();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// ------------------------------------------------------------- display -----
void display() {
    glClearColor(0.008f, 0.010f, 0.025f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, static_cast<double>(g.width) / std::max(g.height, 1), 0.1, 500.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    const float yaw = g.yaw * kPi / 180.0f, pitch = g.pitch * kPi / 180.0f;
    gluLookAt(g.distance * std::cos(pitch) * std::sin(yaw), g.distance * std::sin(pitch),
              g.distance * std::cos(pitch) * std::cos(yaw), 0, 0, 0, 0, 1, 0);

    // The star is the light source: a positional light at the origin.
    const GLfloat lightPos[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    if (g.fog) glEnable(GL_FOG); else glDisable(GL_FOG);
    glPolygonMode(GL_FRONT_AND_BACK, g.wireframe ? GL_LINE : GL_FILL);

    const int phase = currentPhase();
    const float t = g.time;

    // Background stars (fixed directions, drawn first, no depth write).
    glDisable(GL_FOG);
    glDepthMask(GL_FALSE);
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    uint32_t s = 12345u;
    for (int i = 0; i < 900; ++i) {
        s = s * 1664525u + 1013904223u; const float u = ((s >> 8) & 0xFFFF) / 65535.0f * 2.0f - 1.0f;
        s = s * 1664525u + 1013904223u; const float a = ((s >> 8) & 0xFFFF) / 65535.0f * 2.0f * kPi;
        s = s * 1664525u + 1013904223u; const float b = 0.3f + 0.7f * ((s >> 8) & 0xFF) / 255.0f;
        const float r = std::sqrt(1.0f - u * u) * 300.0f;
        glColor4f(b, b, b * 1.1f, 1.0f);
        glVertex3f(r * std::cos(a), u * 300.0f, r * std::sin(a));
    }
    glEnd();
    glDepthMask(GL_TRUE);
    if (g.fog) glEnable(GL_FOG);

    // --- Cloud: static display list in phase 1, animated collapse in phase 2-3.
    const float collapse = smooth01(kPhaseSeconds, 2.6f * kPhaseSeconds, t);
    const float cloudFade = 1.0f - smooth01(3.0f * kPhaseSeconds, 4.2f * kPhaseSeconds, t);
    glPointSize(2.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glPushMatrix();
    glRotatef(t * 2.0f, 0, 1, 0);
    if (collapse <= 0.0f) {
        glCallList(g.cloudList);
    } else if (cloudFade > 0.0f) {
        glBegin(GL_POINTS);
        for (const Particle& p : g_cloud) {
            const float r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
            // Inside-out: inner material reaches the center first.
            const float k = std::clamp(collapse * (1.6f - r / 14.0f), 0.0f, 1.0f);
            const float f = 1.0f - k * 0.92f;
            const float spin = k * k * 3.0f;  // angular momentum: spins up as it falls in
            const float x = p.x * f, z = p.z * f;
            const float heat = k;
            glColor4f(0.35f + 0.65f * heat, 0.40f + 0.25f * heat, 0.85f - 0.6f * heat, 0.35f * cloudFade);
            glVertex3f(x * std::cos(spin) - z * std::sin(spin), p.y * f * (1.0f - 0.6f * k),
                       x * std::sin(spin) + z * std::cos(spin));
        }
        glEnd();
    }
    glPopMatrix();
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- Central object: protostar -> main-sequence star -> giant -> white dwarf.
    const float protostar = smooth01(1.8f * kPhaseSeconds, 2.4f * kPhaseSeconds, t);
    const float ignite = smooth01(4.0f * kPhaseSeconds, 4.3f * kPhaseSeconds, t);
    const float giant = smooth01(5.0f * kPhaseSeconds, 5.35f * kPhaseSeconds, t);
    const float shed = smooth01(5.35f * kPhaseSeconds, 5.7f * kPhaseSeconds, t);
    if (protostar > 0.0f && shed < 1.0f) {
        float radius = 1.6f * protostar * (1.0f - 0.35f * ignite);
        float r = 1.0f, gc = 0.55f + 0.35f * ignite, b = 0.25f + 0.6f * ignite;
        radius = radius + (5.5f - radius) * giant;
        gc = gc + (0.5f - gc) * giant;
        b = b + (0.25f - b) * giant;
        radius *= 1.0f - 0.9f * shed;
        drawStar(radius, r, gc, b, 0.75f);
    }
    // An illustrative planet orbiting the main-sequence star. It is the one object
    // lit from outside, so it shows the fixed-function lighting: the day side faces
    // GL_LIGHT0 at the star (diffuse + specular), the night side keeps only ambient.
    const float planet = smooth01(4.2f * kPhaseSeconds, 4.5f * kPhaseSeconds, t) * (1.0f - giant);
    if (planet > 0.0f) {
        const float orbit = 0.35f * t;
        glPushMatrix();
        glTranslatef(9.0f * std::cos(orbit), 0.0f, 9.0f * std::sin(orbit));
        const GLfloat diffuse[] = {0.35f * planet, 0.55f * planet, 0.95f * planet, 1.0f};
        const GLfloat specular[] = {0.6f, 0.6f, 0.6f, 1.0f};
        glEnable(GL_LIGHTING);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 40.0f);
        if (g.wireframe) glutWireSphere(1.1, 20, 14); else glutSolidSphere(1.1, 40, 28);
        glDisable(GL_LIGHTING);
        glPopMatrix();
    }
    drawDisc(smooth01(2.8f * kPhaseSeconds, 3.2f * kPhaseSeconds, t) * (1.0f - smooth01(4.3f * kPhaseSeconds, 4.8f * kPhaseSeconds, t)), 9.0f);
    drawJets(smooth01(3.1f * kPhaseSeconds, 3.4f * kPhaseSeconds, t) * (1.0f - smooth01(4.2f * kPhaseSeconds, 4.5f * kPhaseSeconds, t)));

    // --- Planetary nebula shell (wire sphere, scaled by the matrix stack) + white dwarf.
    if (shed > 0.0f) {
        const float age = t - 5.35f * kPhaseSeconds;
        const float R = 5.5f + age * 1.1f;
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glPushMatrix();
        glScalef(R, R * 1.35f, R);  // bipolar elongation along the axis
        glRotatef(t * 4.0f, 0, 1, 0);
        glColor4f(0.25f, 0.9f, 0.8f, 0.18f * shed);
        glutWireSphere(1.0, 36, 24);
        glColor4f(1.0f, 0.3f, 0.25f, 0.10f * shed);
        glScalef(1.08f, 1.08f, 1.08f);
        glutWireSphere(1.0, 30, 18);
        glPopMatrix();
        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        drawStar(0.35f, 0.75f, 0.85f, 1.0f, 1.0f);  // white dwarf (enlarged)
    }

    // --- 2D overlay (orthographic projection, bitmap text).
    glDisable(GL_FOG);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, g.width, 0, g.height);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(0.95f, 0.78f, 0.45f);
    drawText(20, g.height - 34.0f, "STELLARIS  -  freeGLUT / OpenGL demonstration", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.9f, 0.92f, 1.0f);
    drawText(20, g.height - 60.0f, kPhaseNames[phase], GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.65f, 0.7f, 0.8f);
    drawText(20, g.height - 80.0f, kPhaseNotes[phase]);
    char buf[160];
    std::snprintf(buf, sizeof(buf), "t = %5.1f s   speed %.2fx   %s   phase %d/%d", t, g.speed,
                  g.paused ? "[paused]" : "", phase + 1, kPhaseCount);
    drawText(20, g.height - 100.0f, buf);
    // Timeline bar.
    const float bx = 20, by = 24, bw = g.width - 40.0f;
    glBegin(GL_QUADS);
    for (int i = 0; i < kPhaseCount; ++i) {
        const float x0 = bx + bw * i / kPhaseCount + 2, x1 = bx + bw * (i + 1) / kPhaseCount - 2;
        const float on = (i == phase) ? 0.75f : 0.25f;
        glColor4f(0.6f, 0.65f, 0.95f, on);
        glVertex2f(x0, by); glVertex2f(x1, by); glVertex2f(x1, by + 6); glVertex2f(x0, by + 6);
    }
    const float px = bx + bw * std::min(t / (kPhaseCount * kPhaseSeconds), 1.0f);
    glColor4f(1, 1, 1, 1);
    glVertex2f(px - 2, by - 4); glVertex2f(px + 2, by - 4); glVertex2f(px + 2, by + 10); glVertex2f(px - 2, by + 10);
    glEnd();
    if (g.showHelp) {
        glColor3f(0.55f, 0.6f, 0.72f);
        const char* help[] = {"Space pause   N / P next / previous phase   1-6 jump   + / - speed   R reset",
                              "Drag orbit   Wheel zoom   W wireframe   G fog   F full screen   H help   Right-click menu   Esc quit"};
        drawText(20, 56, help[0]);
        drawText(20, 40, help[1]);
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);

    glutSwapBuffers();
}

// ------------------------------------------------------- PNG (capture) -----
bool savePng(const std::string& path) {
    const int w = g.width, h = g.height;
    std::vector<unsigned char> px(static_cast<size_t>(w) * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    std::vector<unsigned char> raw;
    for (int y = h - 1; y >= 0; --y) {
        raw.push_back(0);
        raw.insert(raw.end(), px.begin() + static_cast<size_t>(y) * w * 3, px.begin() + static_cast<size_t>(y + 1) * w * 3);
    }
    auto crc = [](const unsigned char* d, size_t n) {
        uint32_t c = 0xFFFFFFFFu;
        for (size_t i = 0; i < n; ++i) { c ^= d[i]; for (int k = 0; k < 8; ++k) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u))); }
        return ~c;
    };
    auto be32 = [](std::vector<unsigned char>& v, uint32_t x) { for (int s = 24; s >= 0; s -= 8) v.push_back(static_cast<unsigned char>(x >> s)); };
    std::vector<unsigned char> z = {0x78, 0x01};
    uint32_t a = 1, b = 0;
    for (unsigned char c : raw) { a = (a + c) % 65521; b = (b + a) % 65521; }
    for (size_t p = 0; p < raw.size();) {
        const size_t n = std::min<size_t>(65535, raw.size() - p);
        z.push_back(p + n == raw.size() ? 1 : 0);
        z.push_back(n & 0xFF); z.push_back((n >> 8) & 0xFF); z.push_back(~n & 0xFF); z.push_back((~n >> 8) & 0xFF);
        z.insert(z.end(), raw.begin() + p, raw.begin() + p + n);
        p += n;
    }
    be32(z, (b << 16) | a);
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    auto chunk = [&](const char* type, const std::vector<unsigned char>& data) {
        std::vector<unsigned char> c;
        be32(c, static_cast<uint32_t>(data.size()));
        c.insert(c.end(), type, type + 4);
        c.insert(c.end(), data.begin(), data.end());
        be32(c, crc(c.data() + 4, c.size() - 4));
        out.write(reinterpret_cast<const char*>(c.data()), static_cast<std::streamsize>(c.size()));
    };
    out.write("\x89PNG\r\n\x1a\n", 8);
    std::vector<unsigned char> ihdr;
    be32(ihdr, static_cast<uint32_t>(w)); be32(ihdr, static_cast<uint32_t>(h));
    ihdr.insert(ihdr.end(), {8, 2, 0, 0, 0});
    chunk("IHDR", ihdr);
    chunk("IDAT", z);
    chunk("IEND", {});
    return static_cast<bool>(out);
}

// ----------------------------------------------------------- callbacks -----
void reshape(int w, int h) {
    g.width = std::max(w, 1);
    g.height = std::max(h, 1);
    glViewport(0, 0, g.width, g.height);
}

void timer(int) {
    // Capture only after the window has been shown and drawn a few times:
    // pixels of a not-yet-visible window are undefined when read back.
    static int ticks = 0;
    if (!g.capturePath.empty() && ++ticks >= 20) {
        display();  // render into the back buffer, then read it
        const bool ok = savePng(g.capturePath);
        std::printf("%s %s\n", ok ? "Captured" : "FAILED to capture", g.capturePath.c_str());
        glutLeaveMainLoop();
        return;
    }
    if (!g.paused) {
        g.time += g.speed / 60.0f;
        if (g.time > kPhaseCount * kPhaseSeconds) g.time = 0.0f;  // loop the demonstration
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void reset() {
    g.time = 0.0f; g.speed = 1.0f; g.paused = false;
    g.yaw = 25.0f; g.pitch = 18.0f; g.distance = 42.0f;
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
    case 27: glutLeaveMainLoop(); break;
    case ' ': g.paused = !g.paused; break;
    case 'n': case 'N': g.time = std::min((currentPhase() + 1) * kPhaseSeconds, kPhaseCount * kPhaseSeconds - 0.01f); break;
    case 'p': case 'P': g.time = std::max((currentPhase() - 1) * kPhaseSeconds, 0.0f); break;
    case '+': case '=': g.speed = std::min(g.speed * 2.0f, 8.0f); break;
    case '-': case '_': g.speed = std::max(g.speed * 0.5f, 0.125f); break;
    case 'r': case 'R': reset(); break;
    case 'w': case 'W': g.wireframe = !g.wireframe; break;
    case 'g': case 'G': g.fog = !g.fog; break;
    case 'h': case 'H': g.showHelp = !g.showHelp; break;
    case 'f': case 'F': glutFullScreenToggle(); break;
    default:
        if (key >= '1' && key <= '6') g.time = (key - '1') * kPhaseSeconds;
        break;
    }
    glutPostRedisplay();
}

void special(int key, int, int) {
    if (key == GLUT_KEY_LEFT) g.yaw -= 5.0f;
    if (key == GLUT_KEY_RIGHT) g.yaw += 5.0f;
    if (key == GLUT_KEY_UP) g.pitch = std::min(g.pitch + 5.0f, 85.0f);
    if (key == GLUT_KEY_DOWN) g.pitch = std::max(g.pitch - 5.0f, -85.0f);
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        g.dragging = state == GLUT_DOWN;
        g.dragX = x; g.dragY = y;
    }
}

void motion(int x, int y) {
    if (!g.dragging) return;
    g.yaw += (x - g.dragX) * 0.3f;
    g.pitch = std::clamp(g.pitch + (y - g.dragY) * 0.3f, -85.0f, 85.0f);
    g.dragX = x; g.dragY = y;
    glutPostRedisplay();
}

void wheel(int, int dir, int, int) {
    g.distance = std::clamp(g.distance * (dir > 0 ? 0.9f : 1.1f), 10.0f, 150.0f);
    glutPostRedisplay();
}

enum MenuItem { kMenuPause = 100, kMenuReset, kMenuWire, kMenuFog, kMenuFull, kMenuQuit };
void menu(int item) {
    if (item >= 0 && item < kPhaseCount) g.time = item * kPhaseSeconds;
    switch (item) {
    case kMenuPause: g.paused = !g.paused; break;
    case kMenuReset: reset(); break;
    case kMenuWire: g.wireframe = !g.wireframe; break;
    case kMenuFog: g.fog = !g.fog; break;
    case kMenuFull: glutFullScreenToggle(); break;
    case kMenuQuit: glutLeaveMainLoop(); break;
    default: break;
    }
    glutPostRedisplay();
}

// Called by freeGLUT while the window (and its GL context) still exists.
void cleanup() {
    if (g.cloudList) glDeleteLists(g.cloudList, 1);
    g.cloudList = 0;
}

void initGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHT0);
    const GLfloat warm[] = {1.0f, 0.85f, 0.6f, 1.0f};
    const GLfloat ambient[] = {0.05f, 0.05f, 0.08f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, warm);
    glLightfv(GL_LIGHT0, GL_SPECULAR, warm);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    // Exponential fog darkens distant geometry: a classic depth cue.
    const GLfloat fogColor[] = {0.008f, 0.010f, 0.025f, 1.0f};
    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.012f);
}

} // namespace

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--capture") && i + 2 < argc) {
            g.capturePath = argv[++i];
            g.time = static_cast<float>(std::atof(argv[++i]));
            g.paused = true;
        }
    }
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(g.width, g.height);
    glutCreateWindow("STELLARIS - freeGLUT / OpenGL demonstration");
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    std::printf("OpenGL %s | %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));

    initGL();
    buildCloud();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMouseWheelFunc(wheel);
    glutTimerFunc(16, timer, 0);
    glutCloseFunc(cleanup);

    const int phases = glutCreateMenu(menu);
    for (int i = 0; i < kPhaseCount; ++i) glutAddMenuEntry(kPhaseNames[i], i);
    glutCreateMenu(menu);
    glutAddSubMenu("Jump to phase", phases);
    glutAddMenuEntry("Pause / resume", kMenuPause);
    glutAddMenuEntry("Reset", kMenuReset);
    glutAddMenuEntry("Toggle wireframe", kMenuWire);
    glutAddMenuEntry("Toggle fog", kMenuFog);
    glutAddMenuEntry("Full screen", kMenuFull);
    glutAddMenuEntry("Quit", kMenuQuit);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();
    return 0;
}
