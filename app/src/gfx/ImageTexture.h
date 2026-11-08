#pragma once
// Loads a JPEG/PNG file into a mipmapped RGBA8 texture for display (UI images).
// Pixel values are uploaded unchanged (sRGB-encoded), because the interface is
// drawn straight to the non-sRGB default framebuffer.

#include "gfx/GL.h"

#include <filesystem>
#include <optional>

namespace stellaris {

struct ImageTexture {
    gl::Texture texture;
    int width = 0, height = 0;
};

// srgb = true stores the texture as GL_SRGB8_ALPHA8 so shaders sample linear values
// (used for the sky photograph rendered into the linear HDR target).
std::optional<ImageTexture> loadImageTexture(const std::filesystem::path& path, bool srgb = false);

} // namespace stellaris
