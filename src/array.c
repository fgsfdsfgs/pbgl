#include <pbkit/pbkit.h>
#include <string.h>

#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "error.h"
#include "state.h"
#include "misc.h"
#include "push.h"
#include "array.h"

static inline GLuint varr_to_nvarr(const GLenum arr) {
  static const GLuint arrmap[] = {
    0,                         // VARR_INDEX // TODO
    NV2A_VERTEX_ATTR_POSITION, // VARR_POSITION
    NV2A_VERTEX_ATTR_NORMAL,   // VARR_NORMAL
    NV2A_VERTEX_ATTR_TEXTURE0, // VARR_TEXCOORD
    NV2A_VERTEX_ATTR_DIFFUSE,  // VARR_COLOR1
    NV2A_VERTEX_ATTR_SPECULAR, // VARR_COLOR2
    NV2A_VERTEX_ATTR_FOG,      // VARR_FOG
  };
  return arrmap[arr];
}

static inline GLuint gltype_to_nvtype(const GLenum gltype) {
  switch(gltype) {
    case GL_FLOAT:         return NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F;
    case GL_SHORT:         return NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S32K;
    case GL_UNSIGNED_BYTE: return NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_OGL;
    default:               return 0;
  }
}

static inline void array_set(const GLenum index, const GLenum type, const GLsizei size, const GLsizei stride, const void *data) {
  varray_state_t *arr;

  // don't forget about multitexturing
  if (index == VARR_TEXCOORD)
    arr = &pbgl.tex[pbgl.active_tex_cl].varray;
  else
    arr = &pbgl.varray[index];

  arr->type = type;
  arr->size = size;
  arr->stride = stride;
  arr->data = data;
}

static inline GLuint *array_set_data_format(GLuint *p, const GLuint idx, const GLenum type, const GLuint size, const GLuint stride) {
  return push_command_parameter(p, NV097_SET_VERTEX_DATA_ARRAY_FORMAT + idx * 4,
    PBGL_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE, type) |
    PBGL_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_SIZE, size) |
    PBGL_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_STRIDE, stride));
}

static inline GLuint *array_flush(GLuint *p, const varray_state_t *arr, const GLuint target)  {
  if (arr->enabled) {
    const GLuint nvtype = gltype_to_nvtype(arr->type);
    if (nvtype) {
      p = array_set_data_format(p, target, nvtype, arr->size, arr->stride);
      p = push_command_parameter(p, NV097_SET_VERTEX_DATA_ARRAY_OFFSET + target * 4, (GLuint)arr->data & 0x03FFFFF);
    }
  } else {
    // if the array is disabled, make sure we set the current value at least
    p = push_command(p, NV097_SET_VERTEX_DATA4F_M + target * 4 * 4, 4);
    p = push_float(p, arr->value.x);
    p = push_float(p, arr->value.y);
    p = push_float(p, arr->value.z);
    p = push_float(p, arr->value.w);
  }
  return p;
}

// these are basically taken from xgux

static inline GLuint *push_elements16(GLuint *p, const GLushort *elements, const GLuint count) {
  p = push_command(p, 0x40000000 | NV097_ARRAY_ELEMENT16, count / 2);
  memcpy(p, elements, count * sizeof(GLushort));
  return p + count / 2;
}

static inline GLuint *push_elements32(GLuint *p, const GLuint *elements, const GLuint count) {
  p = push_command(p, 0x40000000 | NV097_ARRAY_ELEMENT32, count);
  memcpy(p, elements, count * sizeof(GLuint));
  return p + count;
}

static inline GLuint *draw_elements16(GLuint *p, const GLushort *elements, GLuint count) {
  const GLuint pair_count = count / 2;
  GLuint i = 0;

  while (i < pair_count) {
    // start next batch
    pb_end(p);
    p = pb_begin();
    const GLuint batch_pair_count = umin(pair_count - i, ARRAY_MAX_BATCH);
    p = push_elements16(p, &elements[i * 2], batch_pair_count * 2);
    i += batch_pair_count;
  }

  // submit final index if necessary
  if (count & 1) {
    const GLuint index = elements[count - 1];
    p = push_elements32(p, &index, 1);
  }

  return p;
}

static inline GLuint *draw_elements32(GLuint *p, const GLuint *elements, GLuint count) {
  while (count > 0) {
    // start next batch
    pb_end(p);
    p = pb_begin();
    const GLuint batch_count = umin(count, ARRAY_MAX_BATCH);
    p = push_elements32(p, elements, batch_count);
    elements += batch_count;
    count -= batch_count;
  }
  return p;
}

/* pbgl internals */

void pbgl_array_reset_all(void) {
  GLuint *p = pb_begin();
  for(GLuint i = 0; i < ARRAY_NV_COUNT; i++)
    p = array_set_data_format(p, i, NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F, 0, 0);
  pb_end(p);
}

void pbgl_array_flush_all(void) {
  // reset all vertex attribs
  pbgl_array_reset_all();

  // set the ones that are enabled, for ones that aren't specify current immediate value
  GLuint *p = pb_begin();

  // set the "regular" arrays
  for (GLuint i = 1; i < VARR_COUNT; ++i) {
    // TODO: maybe structure the varray array better so we wouldn't have to do this shit
    if (i == VARR_TEXCOORD) continue;
    p = array_flush(p, &pbgl.varray[i], varr_to_nvarr(i));
  }

  // set the texcoord arrays
  for (GLuint i = 0; i < TEXUNIT_COUNT; ++i) {
    if (pbgl.tex[i].enabled)
      p = array_flush(p, &pbgl.tex[i].varray, NV2A_VERTEX_ATTR_TEXTURE0 + i);
  }

  pb_end(p);
}

/* GL FUNCTIONS BEGIN */

GL_API void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
  array_set(VARR_POSITION, type, size, stride, pointer);
}

GL_API void glNormalPointer(GLenum type, GLsizei stride, const void *pointer) {
  array_set(VARR_NORMAL, type, 3, stride, pointer);
}

GL_API void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
  array_set(VARR_TEXCOORD, type, size, stride, pointer);
}

GL_API void glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
  array_set(VARR_COLOR1, type, size, stride, pointer);
}

GL_API void glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
  array_set(VARR_COLOR2, type, size, stride, pointer);
}

GL_API void glFogCoordPointer(GLenum type, GLsizei stride, const void *pointer) {
  array_set(VARR_FOG, type, 1, stride, pointer);
}

GL_API void glDrawArrays(GLenum prim, GLint first, GLsizei count) {
  if (pbgl.imm.active || !count) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  // NV2A prim type values are the same as GL prim types,
  // but with 1 added to accound for OP_END = 0, and they go from 1 to A
  if (prim >= 0xA) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  ++prim;

  pbgl_state_flush();
  pbgl_array_flush_all();

  // this is basically xgux_draw_arrays

  GLuint *p = pb_begin();
  p = push_command_parameter(p, NV097_SET_BEGIN_END, prim);

  while(count > 0) {
    // start next batch
    pb_end(p);
    p = pb_begin();

    const GLint batch_count = imin(count, ARRAY_MAX_BATCH);
    p = push_command_parameter(p, 0x40000000 | NV097_DRAW_ARRAYS,
      PBGL_MASK(NV097_DRAW_ARRAYS_COUNT, (count - 1)) |
      PBGL_MASK(NV097_DRAW_ARRAYS_START_INDEX, first));

    first += batch_count;
    count -= batch_count;
  }

  p = push_command_parameter(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
  pb_end(p);
}

GL_API void glDrawElements(GLenum prim, GLsizei count, GLenum idxtype, const void *indices) {
  if (pbgl.imm.active || !count) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  if (prim++ >= 0xA) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  pbgl_state_flush();
  pbgl_array_flush_all();

  GLuint *p = pb_begin();
  p = push_command_parameter(p, NV097_SET_BEGIN_END, prim);

  if (idxtype == GL_UNSIGNED_SHORT)
    p = draw_elements16(p, indices, count);
  else if (idxtype == GL_UNSIGNED_INT)
    p = draw_elements32(p, indices, count);
  else
    pbgl_set_error(GL_INVALID_ENUM);

  p = push_command_parameter(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
  pb_end(p);
}
