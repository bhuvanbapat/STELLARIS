#include "core/PngWriter.h"

#include <algorithm>
#include <array>
#include <fstream>

namespace stellaris {

namespace {

uint32_t crc32(const uint8_t* data, size_t len, uint32_t crc = 0) {
    static const std::array<uint32_t, 256> table = [] {
        std::array<uint32_t, 256> t{};
        for (uint32_t n = 0; n < 256; ++n) {
            uint32_t c = n;
            for (int k = 0; k < 8; ++k) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            t[n] = c;
        }
        return t;
    }();
    crc = ~crc;
    for (size_t i = 0; i < len; ++i) crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return ~crc;
}

void putU32(std::vector<uint8_t>& v, uint32_t x) {
    v.insert(v.end(), {uint8_t(x >> 24), uint8_t(x >> 16), uint8_t(x >> 8), uint8_t(x)});
}

void writeChunk(std::ofstream& out, const char type[4], const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> buf;
    putU32(buf, static_cast<uint32_t>(payload.size()));
    buf.insert(buf.end(), type, type + 4);
    buf.insert(buf.end(), payload.begin(), payload.end());
    putU32(buf, crc32(buf.data() + 4, buf.size() - 4));
    out.write(reinterpret_cast<const char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
}

} // namespace

bool writePng(const std::filesystem::path& path, int width, int height, const std::vector<uint8_t>& rgb) {
    if (width <= 0 || height <= 0 || rgb.size() != static_cast<size_t>(width) * height * 3) return false;

    // Raw scanlines, each prefixed with filter type 0.
    std::vector<uint8_t> raw;
    const size_t stride = static_cast<size_t>(width) * 3;
    raw.reserve((stride + 1) * height);
    for (int y = 0; y < height; ++y) {
        raw.push_back(0);
        raw.insert(raw.end(), rgb.begin() + y * stride, rgb.begin() + (y + 1) * stride);
    }

    // zlib stream with stored (uncompressed) deflate blocks of <= 65535 bytes.
    std::vector<uint8_t> z = {0x78, 0x01};
    uint32_t a = 1, b = 0;
    for (uint8_t byte : raw) {
        a = (a + byte) % 65521;
        b = (b + a) % 65521;
    }
    for (size_t pos = 0; pos < raw.size();) {
        const size_t n = std::min<size_t>(65535, raw.size() - pos);
        const bool last = pos + n == raw.size();
        z.push_back(last ? 1 : 0);
        z.insert(z.end(), {uint8_t(n), uint8_t(n >> 8), uint8_t(~n), uint8_t(~n >> 8)});
        z.insert(z.end(), raw.begin() + pos, raw.begin() + pos + n);
        pos += n;
    }
    putU32(z, (b << 16) | a);

    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    const uint8_t signature[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    out.write(reinterpret_cast<const char*>(signature), 8);
    std::vector<uint8_t> ihdr;
    putU32(ihdr, static_cast<uint32_t>(width));
    putU32(ihdr, static_cast<uint32_t>(height));
    ihdr.insert(ihdr.end(), {8, 2, 0, 0, 0});  // 8-bit, truecolor, deflate, no filter, no interlace
    writeChunk(out, "IHDR", ihdr);
    writeChunk(out, "IDAT", z);
    writeChunk(out, "IEND", {});
    return static_cast<bool>(out);
}

} // namespace stellaris
