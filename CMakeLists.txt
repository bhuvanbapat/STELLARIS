cmake_minimum_required(VERSION 3.24)

project(STELLARIS
    VERSION 1.0.0
    DESCRIPTION "Interactive cinematic visualization of stellar formation and evolution"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(STELLARIS_BUILD_TESTS     "Build the STELLARIS unit tests"              ON)
option(STELLARIS_BUILD_GLUT_DEMO "Build the freeGLUT demonstration program"   ON)

# Put every executable in one predictable folder: build/bin/<Config>/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(cmake/Dependencies.cmake)

add_subdirectory(third_party/glad)
add_subdirectory(app)

if(STELLARIS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(STELLARIS_BUILD_GLUT_DEMO)
    add_subdirectory(glut_demo)
endif()
