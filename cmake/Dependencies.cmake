# Third-party dependencies for the main STELLARIS application.
#
# Every dependency is pinned to a released tag and downloaded once at configure
# time by CMake's FetchContent. After the first configure, the sources live in
# build/_deps and no network access is needed to build or run the application.
#
#   GLFW  3.4     window, OpenGL context, input
#   GLM   1.0.1   vector / matrix math
#   ImGui 1.91.5  immediate-mode user interface
#   stb_image 2.30 JPEG decoding for attributed observation images (pinned commit + SHA-256)
#   miniaudio 0.11.25 audio playback for the sonification (pinned tag + SHA-256)
#
# The OpenGL loader (glad) is generated once and vendored in third_party/glad.

include(FetchContent)
set(FETCHCONTENT_QUIET ON)

# --- GLFW -------------------------------------------------------------------
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

# --- GLM --------------------------------------------------------------------
set(GLM_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glm
    URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

# --- Dear ImGui (no CMake project upstream; populated only) -----------------
FetchContent_Declare(imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.5.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

FetchContent_MakeAvailable(glfw glm imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw)
target_compile_definitions(imgui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)

# --- stb_image v2.30 (JPEG/PNG decoding for the observation view) ----------
# Single header, MIT or public domain (choice of licence stated in the file).
FetchContent_Declare(stb_image
    URL https://raw.githubusercontent.com/nothings/stb/4e9b368bfc071e97206ea5c6adba68dd0cbf8897/stb_image.h
    URL_HASH SHA256=594c2fe35d49488b4382dbfaec8f98366defca819d916ac95becf3e75f4200b3
    DOWNLOAD_NO_EXTRACT TRUE)
FetchContent_MakeAvailable(stb_image)
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_image_SOURCE_DIR})

# --- miniaudio 0.11.25 (audio output for the sonification) --------------------
# Single header, public domain (Unlicense) or MIT No Attribution, at the user's choice.
FetchContent_Declare(miniaudio
    URL https://raw.githubusercontent.com/mackron/miniaudio/0.11.25/miniaudio.h
    URL_HASH SHA256=ac7af4de748b7e26b777f37e01cee313a308a7296a3eb080e2906b320cc55c89
    DOWNLOAD_NO_EXTRACT TRUE)
FetchContent_MakeAvailable(miniaudio)
add_library(miniaudio INTERFACE)
target_include_directories(miniaudio SYSTEM INTERFACE ${miniaudio_SOURCE_DIR})
