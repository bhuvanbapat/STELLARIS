# --- Core library: pure simulation/model code, no OpenGL (unit-tested) --------
add_library(stellaris_core STATIC
    src/sim/Timeline.cpp
    src/sim/StellarModel.cpp
    src/sim/SceneState.cpp
    src/sim/CameraPath.cpp
    src/sim/StageContent.cpp
    src/sim/CloudField.cpp
    src/sim/MagneticField.cpp
    src/sim/ObservationCatalog.cpp
    src/sim/Sonification.cpp)
target_include_directories(stellaris_core PUBLIC src)
target_link_libraries(stellaris_core PUBLIC glm::glm)

# --- Main application ------------------------------------------------------
add_executable(stellaris
    src/main.cpp
    src/app/App.cpp
    src/audio/AudioEngine.cpp
    src/core/Log.cpp
    src/core/PngWriter.cpp
    src/gfx/GL.cpp
    src/gfx/Shader.cpp
    src/gfx/Geometry.cpp
    src/gfx/ImageTexture.cpp
    src/render/FieldLines.cpp
    src/render/ParticleSystem.cpp
    src/render/Renderer.cpp
    src/ui/Ui.cpp)
target_link_libraries(stellaris PRIVATE stellaris_core glad glfw imgui stb_image miniaudio opengl32)

foreach(target stellaris_core stellaris)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8 /permissive-)
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endif()
endforeach()

# Shaders are loaded at runtime from <exe dir>/shaders (F5 reloads them).
add_custom_command(TARGET stellaris POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:stellaris>/shaders
    COMMENT "Copying shaders next to the executable")
# Attributed images and their provenance sidecars (assets/observations, assets/sky).
add_custom_command(TARGET stellaris POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:stellaris>/assets
    COMMENT "Copying attributed image assets next to the executable")

# Cas A 3D reconstruction -> 192^3 emission texture (tools/bake_casa.py, needs numpy).
find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
    set(CASA_ZIP ${PROJECT_SOURCE_DIR}/assets/data/casa/CasA_supernova_remnant-ascii_vtks.zip)
    set(CASA_OUT ${CMAKE_BINARY_DIR}/casa)
    add_custom_command(OUTPUT ${CASA_OUT}/casa_volume.raw
        COMMAND Python3::Interpreter ${PROJECT_SOURCE_DIR}/tools/bake_casa.py ${CASA_ZIP} ${CASA_OUT} 192
        DEPENDS ${CASA_ZIP} ${PROJECT_SOURCE_DIR}/tools/bake_casa.py
        COMMENT "Baking the Cas A 3D reconstruction into a 3D texture")
    add_custom_target(casa_volume DEPENDS ${CASA_OUT}/casa_volume.raw)
    add_dependencies(stellaris casa_volume)
    add_custom_command(TARGET stellaris POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CASA_OUT}/casa_volume.raw ${CASA_OUT}/casa_volume.json
                $<TARGET_FILE_DIR:stellaris>/assets/data/casa/
        COMMENT "Copying the baked remnant volume")
else()
    message(WARNING "Python 3 not found: the Cas A remnant volume is not baked (procedural fallback)")
endif()

set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT stellaris)
