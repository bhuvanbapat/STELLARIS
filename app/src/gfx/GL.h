#pragma once
// OpenGL loader include plus small move-only RAII wrappers for GL object names,
// so every GPU resource is released exactly once.

#include <glad/gl.h>

#include <utility>

namespace stellaris::gl {

// Owns one GL object name; Deleter is a function that frees it.
template <void (*Deleter)(GLuint)>
class Handle {
public:
    Handle() = default;
    explicit Handle(GLuint id) : id_(id) {}
    ~Handle() { release(); }

    Handle(const Handle&)            = delete;
    Handle& operator=(const Handle&) = delete;
    Handle(Handle&& o) noexcept : id_(std::exchange(o.id_, 0)) {}
    Handle& operator=(Handle&& o) noexcept {
        if (this != &o) {
            release();
            id_ = std::exchange(o.id_, 0);
        }
        return *this;
    }

    GLuint id() const { return id_; }
    explicit operator bool() const { return id_ != 0; }

    void release() {
        if (id_ != 0) Deleter(id_);
        id_ = 0;
    }

private:
    GLuint id_ = 0;
};

namespace detail {
inline void deleteBuffer(GLuint id) { glDeleteBuffers(1, &id); }
inline void deleteVertexArray(GLuint id) { glDeleteVertexArrays(1, &id); }
inline void deleteTexture(GLuint id) { glDeleteTextures(1, &id); }
inline void deleteFramebuffer(GLuint id) { glDeleteFramebuffers(1, &id); }
inline void deleteProgram(GLuint id) { glDeleteProgram(id); }
} // namespace detail

using Buffer      = Handle<detail::deleteBuffer>;
using VertexArray = Handle<detail::deleteVertexArray>;
using Texture     = Handle<detail::deleteTexture>;
using Framebuffer = Handle<detail::deleteFramebuffer>;
using ProgramId   = Handle<detail::deleteProgram>;

inline Buffer createBuffer() { GLuint id = 0; glCreateBuffers(1, &id); return Buffer(id); }
inline VertexArray createVertexArray() { GLuint id = 0; glCreateVertexArrays(1, &id); return VertexArray(id); }
inline Texture createTexture(GLenum target) { GLuint id = 0; glCreateTextures(target, 1, &id); return Texture(id); }
inline Framebuffer createFramebuffer() { GLuint id = 0; glCreateFramebuffers(1, &id); return Framebuffer(id); }

// Routes GL_KHR_debug messages (core in 4.3) to the log. Call after context creation.
void installDebugOutput();

} // namespace stellaris::gl
