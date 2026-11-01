#include "gfx/Shader.h"

#include "core/Log.h"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>

namespace stellaris {

namespace {

bool readFile(const std::filesystem::path& path, std::string& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::ostringstream ss;
    ss << in.rdbuf();
    out = ss.str();
    return true;
}

// Expands `#include "x"` lines (one level deep is enough for this project, but
// recursion is supported with a depth guard).
bool preprocess(const std::filesystem::path& dir, const std::string& file, std::string& out, int depth = 0) {
    if (depth > 8) {
        log::error("Shader include depth exceeded at {}", file);
        return false;
    }
    std::string src;
    if (!readFile(dir / file, src)) {
        log::error("Cannot read shader file: {}", (dir / file).string());
        return false;
    }
    std::istringstream lines(src);
    std::string line;
    while (std::getline(lines, line)) {
        const auto pos = line.find("#include");
        if (pos != std::string::npos && line.find_first_not_of(" \t") == pos) {
            const auto q1 = line.find('"', pos);
            const auto q2 = line.find('"', q1 + 1);
            if (q1 == std::string::npos || q2 == std::string::npos) {
                log::error("Malformed #include in {}: {}", file, line);
                return false;
            }
            if (!preprocess(dir, line.substr(q1 + 1, q2 - q1 - 1), out, depth + 1)) return false;
            out += "\n";
        } else {
            out += line;
            out += "\n";
        }
    }
    return true;
}

GLuint compile(GLenum type, const std::string& source, const std::string& label) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::string infoLog(static_cast<size_t>(len > 1 ? len : 1), '\0');
        glGetShaderInfoLog(shader, len, nullptr, infoLog.data());
        log::error("Shader compile failed ({}):\n{}", label, infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

} // namespace

Shader::Shader(std::string name, std::vector<Stage> stages)
    : name_(std::move(name)), stages_(std::move(stages)) {}

bool Shader::load(const std::filesystem::path& shaderDir) {
    std::vector<GLuint> compiled;
    bool ok = true;
    for (const Stage& stage : stages_) {
        std::string source;
        if (!preprocess(shaderDir, stage.file, source)) { ok = false; break; }
        GLuint s = compile(stage.type, source, name_ + ":" + stage.file);
        if (!s) { ok = false; break; }
        compiled.push_back(s);
    }

    GLuint program = 0;
    if (ok) {
        program = glCreateProgram();
        for (GLuint s : compiled) glAttachShader(program, s);
        glLinkProgram(program);
        GLint linked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint len = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
            std::string infoLog(static_cast<size_t>(len > 1 ? len : 1), '\0');
            glGetProgramInfoLog(program, len, nullptr, infoLog.data());
            log::error("Shader link failed ({}):\n{}", name_, infoLog);
            glDeleteProgram(program);
            program = 0;
            ok = false;
        }
    }
    for (GLuint s : compiled) glDeleteShader(s);

    if (!ok) return false;
    program_ = gl::ProgramId(program);
    locations_.clear();
    return true;
}

GLint Shader::location(const char* name) {
    auto it = locations_.find(name);
    if (it != locations_.end()) return it->second;
    const GLint loc = glGetUniformLocation(program_.id(), name);
    locations_.emplace(name, loc);
    return loc;
}

void Shader::set(const char* n, int v) { glProgramUniform1i(program_.id(), location(n), v); }
void Shader::set(const char* n, float v) { glProgramUniform1f(program_.id(), location(n), v); }
void Shader::set(const char* n, const glm::vec2& v) { glProgramUniform2fv(program_.id(), location(n), 1, glm::value_ptr(v)); }
void Shader::set(const char* n, const glm::vec3& v) { glProgramUniform3fv(program_.id(), location(n), 1, glm::value_ptr(v)); }
void Shader::set(const char* n, const glm::vec4& v) { glProgramUniform4fv(program_.id(), location(n), 1, glm::value_ptr(v)); }
void Shader::set(const char* n, const glm::mat4& v) {
    glProgramUniformMatrix4fv(program_.id(), location(n), 1, GL_FALSE, glm::value_ptr(v));
}

} // namespace stellaris
