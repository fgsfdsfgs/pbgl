#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <pbkit/pbkit.h>

#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "matrix.h"
#include "texture.h"
#include "array.h"
#include "error.h"
#include "texenv.h"
#include "push.h"
#include "state.h"
#include "misc.h"
#include "pbgl.h"

static GLboolean pbkit_initialized = GL_FALSE;
static GLboolean pbkit_do_deinit = GL_FALSE;

// workaround for pbkit enabling the wrong Z buffer mode
static inline void pbgl_target_back_buffer(void) {
  pb_target_back_buffer();
  // override Z-buffer mode set by pbkit's set_draw_buffer() for no apparent reason
  uint32_t *p = pb_begin();
  p = pb_push1(p, NV20_TCL_PRIMITIVE_3D_W_YUV_FPZ_FLAGS, 0x00100001);
  pb_end(p);
}

int pbgl_init(int init_pbkit) {
  if (pbgl.active)
    return -1; // don't double init

  // remember who has to deinit pbkit later
   pbkit_do_deinit = init_pbkit;

  // user says they've already initialized pbkit
  if (!init_pbkit) pbkit_initialized = GL_TRUE;

  // init pbkit if needed
  if (!pbkit_initialized) {
    int err;
    if ((err = pb_init()) != 0)
      return -10 - err;
    pbkit_initialized = GL_TRUE;
  }

  pb_show_front_screen();

  GLuint *p = pb_begin();

  // set fixed pipeline mode
  p = push_command_parameter(p, NV097_SET_TRANSFORM_EXECUTION_MODE,
    PBGL_MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_MODE, NV097_SET_TRANSFORM_EXECUTION_MODE_MODE_FIXED) |
    PBGL_MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE, NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE_PRIV));

  // set default clear parameters that never change (for now)
  p = push_command_parameter(p, NV097_SET_CLEAR_RECT_VERTICAL, (pb_back_buffer_height() << 16));
  p = push_command_parameter(p, NV097_SET_CLEAR_RECT_HORIZONTAL, (pb_back_buffer_width() << 16));

  // set some default crap
  p = push_command_boolean(p, NV097_SET_SPECULAR_ENABLE, 1);
  p = push_command_boolean(p, NV097_SET_NORMALIZATION_ENABLE, 0);
  p = push_command_boolean(p, NV097_SET_ZMIN_MAX_CONTROL, 1);
  p = push_command_boolean(p, NV097_SET_COMPRESS_ZBUFFER_EN, 1);
  p = push_command_boolean(p, NV097_SET_WINDOW_CLIP_TYPE, 0);
  p = push_command_parameter(p, NV097_SET_CONTROL0, NV097_SET_CONTROL0_TEXTUREPERSPECTIVE);
  p = push_command_parameter(p, NV097_SET_SKIN_MODE, NV097_SET_SKIN_MODE_OFF);
  p = push_command_parameter(p, NV097_SET_SHADER_OTHER_STAGE_INPUT,
        PBGL_MASK(NV097_SET_SHADER_OTHER_STAGE_INPUT_STAGE1, 0)
      | PBGL_MASK(NV097_SET_SHADER_OTHER_STAGE_INPUT_STAGE2, 0)
      | PBGL_MASK(NV097_SET_SHADER_OTHER_STAGE_INPUT_STAGE3, 0));
  p = push_command_parameter(p, NV097_SET_LIGHT_LOCAL_RANGE, 0x7149f2ca);

  // push identity viewport parameters
  p = push_command(p, NV097_SET_VIEWPORT_OFFSET, 4);
  p = push_float(p, 0.f);
  p = push_float(p, 0.f);
  p = push_float(p, 0.f);
  p = push_float(p, 0.f);
  p = push_command(p, NV097_SET_VIEWPORT_SCALE, 4);
  p = push_float(p, 1.f);
  p = push_float(p, 1.f);
  p = push_float(p, 1.f);
  p = push_float(p, 1.f);

  // push default z clip range
  p = push_command_float(p, NV097_SET_CLIP_MIN, 0.f);
  p = push_command_float(p, NV097_SET_CLIP_MAX, (GLfloat)PBGL_Z_MAX);

  pb_end(p);

  // push identity matrices everywhere

  p = pb_begin();
  p = push_command_matrix4x4(p, NV097_SET_PROJECTION_MATRIX, mat4_identity.v);
  p = push_command_matrix4x4(p, NV097_SET_MODEL_VIEW_MATRIX, mat4_identity.v);
  p = push_command_matrix4x4(p, NV097_SET_INVERSE_MODEL_VIEW_MATRIX, mat4_identity.v);
  p = push_command_matrix4x4(p, NV097_SET_COMPOSITE_MATRIX, mat4_identity.v);
  pb_end(p);

  p = pb_begin();
  for (GLuint i = 0, ofs = 0; i < TEXUNIT_COUNT; ++i, ofs += 4 * 4 * 4) {
    p = pb_push1(p, NV20_TCL_PRIMITIVE_3D_TX_ENABLE(i), 0);
    p = push_command_matrix4x4(p, NV097_SET_TEXTURE_MATRIX + ofs, mat4_identity.v);
  }
  pb_end(p);

  // clear all the combiner registers that we won't use
  p = pb_begin();
  for (GLuint i = 4; i < 8; ++i) {
    p = pb_push1(p, NV097_SET_COMBINER_COLOR_ICW + i * 4, 0);
    p = pb_push1(p, NV097_SET_COMBINER_COLOR_OCW + i * 4, 0);
    p = pb_push1(p, NV097_SET_COMBINER_ALPHA_ICW + i * 4, 0);
    p = pb_push1(p, NV097_SET_COMBINER_ALPHA_OCW + i * 4, 0);
  }
  pb_end(p);

  while (pb_busy());

  // init texture manager
  if (!pbgl_tex_init()) {
    pb_kill();
    pbkit_initialized = GL_FALSE;
    return -3;
  }

  // HACK: preserve some vars that might've been set before init
  const int swap_interval = pbgl.swap_interval;

  // init default state
  pbgl_state_init();

  // disable all vertex arrays
  pbgl_array_reset_all();

  // flush state right away just in case
  pbgl_state_flush();

  // prepare for drawing the first frame
  pb_reset();
  pbgl_target_back_buffer();

  // wait for anything that's still happening just in case (probably not necessary)
  while (pb_busy());

  // restore vars
  pbgl.swap_interval = swap_interval;

  // we're live
  pbgl.active = GL_TRUE;

  return 0;
}

void pbgl_set_swap_interval(int interval) {
  pbgl.swap_interval = interval;
}

int pbgl_get_swap_interval(void) {
  return pbgl.swap_interval;
}

void pbgl_swap_buffers(void) {
  // swap buffers
  while (pb_busy());
  while (pb_finished());

  // wait up to $swap_interval vblanks
  // same code as in xsm64
  GLint now = pb_get_vbl_counter();
  while ((now - pbgl.last_swap) < pbgl.swap_interval)
      now = pb_wait_for_vbl();
  pbgl.last_swap = now;

  pb_reset();
  pbgl_target_back_buffer();
}

void *pbgl_alloc(unsigned int size, int dynamic) {
  return MmAllocateContiguousMemoryEx(size, 0, PBGL_MAXRAM, 0, dynamic ? 0x404 : 0x04);
}

void pbgl_free(void *addr) {
  if (addr) MmFreeContiguousMemory(addr);
}

void pbgl_shutdown(void) {
  if (!pbgl.active)
    return;

  if (pbkit_initialized) {
    while (pb_busy());
    // only deinit pbkit if it was us who inited it
    if (pbkit_do_deinit) {
      pb_kill();
      pbkit_initialized = GL_FALSE;
      pbkit_do_deinit = GL_FALSE;
    }
  }

  // free all textures
  pbgl_tex_free();

  pbgl.active = GL_FALSE;
}
