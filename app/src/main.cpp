// STELLARIS entry point.
#include "app/App.h"

#include <exception>

#include "core/Log.h"

int main(int argc, char** argv) {
    stellaris::AppOptions options;
    if (!stellaris::parseOptions(argc, argv, options)) return 2;
    try {
        stellaris::App app(std::move(options));
        return app.run();
    } catch (const std::exception& e) {
        stellaris::log::error("Fatal: {}", e.what());
        return 1;
    }
}
