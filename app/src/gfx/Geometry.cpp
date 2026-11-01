#include "gfx/Geometry.h"

#include <cmath>
#include <cstdint>
#include <vector>

namespace stellaris {

namespace {
constexpr float kPi = 3.14159265358979f;

Mesh upload(const std::vector<float>& verts, int floatsPerVertex, const std::vector<uint32_t>& idx) {
    Mesh m;
    m.vao      = gl::createVertexArray();
    m.vertices = gl::createBuffer();
    m.indices  = gl::createBuffer();
    glNamedBufferStorage(m.vertices.id(), static_cast<GLsizeiptr>(verts.size() * sizeof(float)), verts.data(), 0);
    glNamedBufferStorage(m.indices.id(), static_cast<GLsizeiptr>(idx.size() * sizeof(uint32_t)), idx.data(), 0);
    glVertexArrayVertexBuffer(m.vao.id(), 0, m.vertices.id(), 0, floatsPerVertex * static_cast<GLsizei>(sizeof(float)));
    glVertexArrayElementBuffer(m.vao.id(), m.indices.id());
    glEnableVertexArrayAttrib(m.vao.id(), 0);
    glVertexArrayAttribFormat(m.vao.id(), 0, floatsPerVertex, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m.vao.id(), 0, 0);
    m.indexCount = static_cast<GLsizei>(idx.size());
    return m;
}

// Indices for a (cols+1) x (rows+1) vertex grid.
void gridIndices(int cols, int rows, std::vector<uint32_t>& idx) {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const uint32_t a = static_cast<uint32_t>(r * (cols + 1) + c);
            const uint32_t b = a + static_cast<uint32_t>(cols + 1);
            idx.insert(idx.end(), {a, b, a + 1, a + 1, b, b + 1});
        }
    }
}
} // namespace

void Mesh::draw() const {
    glBindVertexArray(vao.id());
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
}

Mesh makeSphere(int slices, int stacks) {
    std::vector<float> v;
    for (int i = 0; i <= stacks; ++i) {
        const float phi = kPi * static_cast<float>(i) / static_cast<float>(stacks);
        for (int j = 0; j <= slices; ++j) {
            const float theta = 2.0f * kPi * static_cast<float>(j) / static_cast<float>(slices);
            v.insert(v.end(), {std::sin(phi) * std::cos(theta), std::cos(phi), std::sin(phi) * std::sin(theta)});
        }
    }
    std::vector<uint32_t> idx;
    gridIndices(slices, stacks, idx);
    return upload(v, 3, idx);
}

Mesh makeAnnulusGrid(int radialSegments, int angularSegments) {
    std::vector<float> v;
    for (int a = 0; a <= angularSegments; ++a) {
        const float angle = 2.0f * kPi * static_cast<float>(a) / static_cast<float>(angularSegments);
        for (int r = 0; r <= radialSegments; ++r) {
            v.insert(v.end(), {static_cast<float>(r) / static_cast<float>(radialSegments), angle});
        }
    }
    std::vector<uint32_t> idx;
    gridIndices(radialSegments, angularSegments, idx);
    return upload(v, 2, idx);
}

Mesh makeCone(int segments, int lengthSegments) {
    std::vector<float> v;
    for (int l = 0; l <= lengthSegments; ++l) {
        const float y = static_cast<float>(l) / static_cast<float>(lengthSegments);
        for (int s = 0; s <= segments; ++s) {
            const float a = 2.0f * kPi * static_cast<float>(s) / static_cast<float>(segments);
            v.insert(v.end(), {y * std::cos(a), y, y * std::sin(a)});
        }
    }
    std::vector<uint32_t> idx;
    gridIndices(segments, lengthSegments, idx);
    return upload(v, 3, idx);
}

} // namespace stellaris
