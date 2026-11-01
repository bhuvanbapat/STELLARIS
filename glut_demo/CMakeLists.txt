# STELLARIS freeGLUT demonstration.
#
# Builds either as part of the main project or on its own:
#   cmake -S glut_demo -B glut_demo/build
#   cmake --build glut_demo/build --config Release
cmake_minimum_required(VERSION 3.24)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(STELLARIS_GLUT_Demo LANGUAGES C CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

if(NOT TARGET freeglut_static)
    include(FetchContent)
    # freeGLUT 3.6.0 declares cmake_minimum_required(3.1); CMake 4 needs this to accept it.
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    set(FREEGLUT_BUILD_DEMOS       OFF CACHE BOOL "" FORCE)
    set(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(FREEGLUT_BUILD_STATIC_LIBS ON  CACHE BOOL "" FORCE)
    set(FREEGLUT_REPLACE_GLUT      OFF CACHE BOOL "" FORCE)
    set(INSTALL_PDB                OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(freeglut
        URL https://github.com/FreeGLUTProject/freeglut/archive/refs/tags/v3.6.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(freeglut)
    unset(CMAKE_POLICY_VERSION_MINIMUM)
endif()

add_executable(stellaris_glut_demo src/main.cpp)
# freeGLUT's CMake does not export its include directory, so add it here.
target_include_directories(stellaris_glut_demo PRIVATE ${freeglut_SOURCE_DIR}/include)
target_link_libraries(stellaris_glut_demo PRIVATE freeglut_static opengl32 glu32 winmm)
if(MSVC)
    target_compile_options(stellaris_glut_demo PRIVATE /W4 /utf-8)
    target_compile_definitions(stellaris_glut_demo PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
