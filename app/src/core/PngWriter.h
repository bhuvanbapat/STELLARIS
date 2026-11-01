#pragma once
// Minimal PNG encoder (8-bit RGB, uncompressed "stored" deflate blocks).
// Files are larger than a compressing encoder would produce, but this avoids a
// third-party dependency for the screenshot feature.

#include <cstdint>
#include <filesystem>
#include <vector>

namespace stellaris {

// rgb: width*height*3 bytes, rows top to bottom.
bool writePng(const std::filesystem::path& path, int width, int height, const std::vector<uint8_t>& rgb);

} // namespace stellaris
