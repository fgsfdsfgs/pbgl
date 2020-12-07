#ifndef _PBGL_MATRIX_H
#define _PBGL_MATRIX_H

#include "types.h"

#define MTX_STACK_SIZE_MV 32
#define MTX_STACK_SIZE_P  4
#define MTX_STACK_SIZE_T  4

enum matrix_type_e {
  MTX_MODELVIEW,
  MTX_PROJECTION,
  MTX_TEXTURE,

  MTX_COUNT
};

void pbgl_mtx_reset(GLenum stack);
mat4f *pbgl_mtx_peek(GLenum stack);
GLint pbgl_mtx_stack_depth(GLenum stack);

#endif // _PBGL_MATRIX_H
