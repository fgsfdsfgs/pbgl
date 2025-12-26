cmake_minimum_required(VERSION 3.16)
project(pbgl LANGUAGES C)

set(PBGL_VERSION 0.1)
set(PBGL_GL_VERSION 1.2)

add_library(pbgl
  src/array.c
  src/error.c
  src/fog.c
  src/immediate.c
  src/info.c
  src/light.c
  src/matrix.c
  src/memory.c
  src/misc.c
  src/pbgl.c
  src/state.c
  src/swizzle.c
  src/texenv.c
  src/texgen.c
  src/texture.c
  src/types.c
)

add_definitions(
  -DXBOX
  -DPBGL_VERSION=0.1
  -DPBGL_GL_VERSION=1.2
)

target_include_directories(pbgl PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
