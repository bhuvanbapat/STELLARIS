#include "gfx/GL.h"

#include "core/Log.h"

namespace stellaris::gl {

namespace {
void GLAD_API_PTR debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                            const GLchar* message, const void* /*user*/) {
    (void)source;
    (void)length;
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
    // 131185: NVIDIA "buffer will use VIDEO memory" informational message.
    // 131204: texture-state notice; 131154: pixel readback sync (screenshots).
    if (id == 131185 || id == 131204 || id == 131154) return;
    if (type == GL_DEBUG_TYPE_ERROR || severity == GL_DEBUG_SEVERITY_HIGH)
        log::error("GL [{}]: {}", id, message);
    else
        log::warn("GL [{}]: {}", id, message);
}
} // namespace

void installDebugOutput() {
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    glEnable(GL_DEBUG_OUTPUT);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugCallback, nullptr);
}

} // namespace stellaris::gl
