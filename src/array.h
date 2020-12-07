#ifndef _PBGL_ARRAY_H
#define _PBGL_ARRAY_H

#include "GL/gl.h"
#include "GL/glext.h"

#define ARRAY_MAX_BATCH 120
#define ARRAY_NV_COUNT 16

enum varray_e {
  VARR_INDEX,    // TODO: color index
  VARR_POSITION,
  VARR_NORMAL,
  VARR_TEXCOORD, // each texunit has its own varray
  VARR_COLOR1,
  VARR_COLOR2,
  VARR_FOG,

  VARR_COUNT
};

#define NV2A_VERTEX_ATTR_POSITION      0
#define NV2A_VERTEX_ATTR_WEIGHT        1
#define NV2A_VERTEX_ATTR_NORMAL        2
#define NV2A_VERTEX_ATTR_DIFFUSE       3
#define NV2A_VERTEX_ATTR_SPECULAR      4
#define NV2A_VERTEX_ATTR_FOG           5
#define NV2A_VERTEX_ATTR_POINT_SIZE    6
#define NV2A_VERTEX_ATTR_BACK_DIFFUSE  7
#define NV2A_VERTEX_ATTR_BACK_SPECULAR 8
#define NV2A_VERTEX_ATTR_TEXTURE0      9
#define NV2A_VERTEX_ATTR_TEXTURE1      10
#define NV2A_VERTEX_ATTR_TEXTURE2      11
#define NV2A_VERTEX_ATTR_TEXTURE3      12
#define NV2A_VERTEX_ATTR_RESERVED1     13
#define NV2A_VERTEX_ATTR_RESERVED2     14
#define NV2A_VERTEX_ATTR_RESERVED3     15

static inline GLenum glarr_to_varr(const GLenum arr) {
  switch (arr) {
    case GL_VERTEX_ARRAY:          return VARR_POSITION;
    case GL_NORMAL_ARRAY:          return VARR_NORMAL;
    case GL_TEXTURE_COORD_ARRAY:   return VARR_TEXCOORD;
    case GL_COLOR_ARRAY:           return VARR_COLOR1;
    case GL_SECONDARY_COLOR_ARRAY: return VARR_COLOR2;
    case GL_FOG_COORD_ARRAY:       return VARR_FOG;
    default:                       return GL_INVALID_ENUM;
  }
}

void pbgl_array_reset_all(void);
void pbgl_array_flush_all(void);

#endif // _PBGL_ARRAY_H
