#pragma once
// GLSL program loaded from files in the shader directory. Supports a simple
// `#include "file.glsl"` directive and live reloading (F5 in the app).

#include "gfx/GL.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace stellaris {

class Shader {
public:
    struct Stage {
        GLenum      type;
        std::string file;  // relative to the shader directory
    };

    Shader() = default;
    Shader(std::string name, std::vector<Stage> stages);

    // Compiles and links. On failure logs the full compiler output and keeps the
    // previously linked program (if any), so a bad live edit does not crash.
    bool load(const std::filesystem::path& shaderDir);

    void use() const { glUseProgram(program_.id()); }
    bool valid() const { return static_cast<bool>(program_); }
    GLuint id() const { return program_.id(); }
    const std::string& name() const { return name_; }

    void set(const char* name, int v);
    void set(const char* name, float v);
    void set(const char* name, const glm::vec2& v);
    void set(const char* name, const glm::vec3& v);
    void set(const char* name, const glm::vec4& v);
    void set(const char* name, const glm::mat4& v);

private:
    GLint location(const char* name);

    std::string                            name_;
    std::vector<Stage>                     stages_;
    gl::ProgramId                          program_;
    std::unordered_map<std::string, GLint> locations_;
};

} // namespace stellaris
