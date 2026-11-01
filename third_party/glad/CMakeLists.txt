# glad 2.0.8 OpenGL 4.5 core loader, generated with:
#   glad --api gl:core=4.5 --extensions GL_EXT_texture_filter_anisotropic --out-path third_party/glad c
add_library(glad STATIC src/gl.c)
target_include_directories(glad PUBLIC include)
