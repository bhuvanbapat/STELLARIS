#include "gfx/ImageTexture.h"

#include "core/Log.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include <stb_image.h>

namespace stellaris {

std::optional<ImageTexture> loadImageTexture(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        log::error("Cannot open image {}", path.string());
        return std::nullopt;
    }
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    int w = 0, h = 0, channels = 0;
    unsigned char* pixels = stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()), &w, &h, &channels, 4);
    if (!pixels) {
        log::error("Cannot decode image {}: {}", path.string(), stbi_failure_reason());
        return std::nullopt;
    }
    ImageTexture img;
    img.width = w;
    img.height = h;
    img.texture = gl::createTexture(GL_TEXTURE_2D);
    const int levels = 1 + static_cast<int>(std::floor(std::log2(std::max(w, h))));
    glTextureStorage2D(img.texture.id(), levels, GL_RGBA8, w, h);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTextureSubImage2D(img.texture.id(), 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateTextureMipmap(img.texture.id());
    glTextureParameteri(img.texture.id(), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(img.texture.id(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(img.texture.id(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(img.texture.id(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    stbi_image_free(pixels);
    log::info("Loaded observation image {} ({}x{}, ~{:.1f} MiB on GPU)", path.filename().string(), w, h,
              w * h * 4.0 * 4.0 / 3.0 / (1024.0 * 1024.0));
    return img;
}

} // namespace stellaris
