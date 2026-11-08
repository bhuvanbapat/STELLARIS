#include "render/FieldLines.h"

#include "core/Log.h"
#include "sim/MagneticField.h"

#include <glm/geometric.hpp>
#include <glm/vec4.hpp>

#include <vector>

namespace stellaris {

namespace {
gl::Buffer upload(const std::vector<std::vector<glm::vec3>>& lines, int& segments) {
    std::vector<glm::vec4> data;
    for (const auto& line : lines) {
        for (size_t i = 0; i + 1 < line.size(); ++i) {
            data.emplace_back(line[i], glm::length(line[i]));
            data.emplace_back(line[i + 1], glm::length(line[i + 1]));
        }
    }
    segments = static_cast<int>(data.size() / 2);
    gl::Buffer b = gl::createBuffer();
    glNamedBufferStorage(b.id(), static_cast<GLsizeiptr>(data.size() * sizeof(glm::vec4)), data.data(), 0);
    return b;
}
} // namespace

void FieldLines::init() {
    const glm::vec3 moment(0.0f, 1.0f, 0.0f);
    // Main-sequence star: a family of closed loops plus a few open polar lines.
    FieldLineSettings star;
    star.maxRadius = 5.0f;
    starBuffer_ = upload(traceDipoleLines(moment, {32.0f, 44.0f, 55.0f, 66.0f, 78.0f}, 8, star), starSegments_);
    // Neutron star (drawn tiny): only high-latitude footpoints, whose loops reach far.
    FieldLineSettings ns;
    ns.maxRadius = 45.0f;
    ns.step = 0.04f;
    ns.maxSteps = 4000;
    nsBuffer_ = upload(traceDipoleLines(moment, {68.0f, 74.0f, 79.0f, 83.0f}, 8, ns), nsSegments_);
    vao_ = gl::createVertexArray();
    log::info("Field lines: {} + {} segments (analytic dipole)", starSegments_, nsSegments_);
}

void FieldLines::draw(Set set) const {
    const bool isStar = set == Set::Star;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, isStar ? starBuffer_.id() : nsBuffer_.id());
    glBindVertexArray(vao_.id());
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, isStar ? starSegments_ : nsSegments_);
}

} // namespace stellaris
