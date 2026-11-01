#include "core/Log.h"

#include <cstdio>
#include <mutex>

namespace stellaris::log {

namespace {
std::FILE* g_file = nullptr;
std::mutex g_mutex;

const char* prefix(Level level) {
    switch (level) {
    case Level::Info:  return "[info ] ";
    case Level::Warn:  return "[warn ] ";
    case Level::Error: return "[error] ";
    }
    return "";
}
} // namespace

void openLogFile(const char* path) {
    std::lock_guard lock(g_mutex);
    if (g_file) std::fclose(g_file);
    g_file = std::fopen(path, "w");
}

void write(Level level, std::string_view message) {
    std::lock_guard lock(g_mutex);
    std::fprintf(stderr, "%s%.*s\n", prefix(level), static_cast<int>(message.size()), message.data());
    if (g_file) {
        std::fprintf(g_file, "%s%.*s\n", prefix(level), static_cast<int>(message.size()), message.data());
        std::fflush(g_file);
    }
}

} // namespace stellaris::log
