# Unit tests for the pure simulation layer (no window or GPU required).
# A tiny built-in harness keeps the project free of an extra test dependency.
add_executable(stellaris_tests
    test_main.cpp
    test_timeline.cpp
    test_stellar_model.cpp
    test_scene_state.cpp
    test_cloud_field.cpp
    test_magnetic_field.cpp
    test_observations.cpp
    test_sonification.cpp
    test_evolution_phase.cpp
    ${PROJECT_SOURCE_DIR}/app/src/render/ParticleSystem.cpp
    ${PROJECT_SOURCE_DIR}/app/src/audio/AudioEngine.cpp
    ${PROJECT_SOURCE_DIR}/app/src/core/Log.cpp)
target_link_libraries(stellaris_tests PRIVATE stellaris_core glad miniaudio)
target_include_directories(stellaris_tests PRIVATE ${PROJECT_SOURCE_DIR}/app/src)
if(MSVC)
    target_compile_options(stellaris_tests PRIVATE /W4 /utf-8)
endif()
target_compile_definitions(stellaris_tests PRIVATE STELLARIS_SOURCE_DIR="${PROJECT_SOURCE_DIR}" _CRT_SECURE_NO_WARNINGS)
add_test(NAME stellaris_unit_tests COMMAND stellaris_tests)

# Provenance of the observation assets: required fields, checksums, dimensions.
find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
    add_test(NAME observation_provenance COMMAND Python3::Interpreter ${PROJECT_SOURCE_DIR}/tools/check_observations.py)
endif()
