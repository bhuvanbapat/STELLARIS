# Unit tests for the pure simulation layer (no window or GPU required).
# A tiny built-in harness keeps the project free of an extra test dependency.
add_executable(stellaris_tests
    test_main.cpp
    test_timeline.cpp
    test_stellar_model.cpp
    test_scene_state.cpp
    test_cloud_field.cpp
    test_magnetic_field.cpp
    ${PROJECT_SOURCE_DIR}/app/src/render/ParticleSystem.cpp)
target_link_libraries(stellaris_tests PRIVATE stellaris_core glad)
target_include_directories(stellaris_tests PRIVATE ${PROJECT_SOURCE_DIR}/app/src)
if(MSVC)
    target_compile_options(stellaris_tests PRIVATE /W4 /utf-8)
endif()
add_test(NAME stellaris_unit_tests COMMAND stellaris_tests)
