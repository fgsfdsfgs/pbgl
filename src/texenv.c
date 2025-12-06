#include <pbkit/pbkit.h>
#include <limits.h>

#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "error.h"
#include "texture.h"
#include "state.h"
#include "misc.h"
#include "texenv.h"

#define RC_ZERO 0x0
#define RC_DISCARD 0x0
#define RC_CONSTANT_COLOR0 0x1
#define RC_CONSTANT_COLOR1 0x2
#define RC_PRIMARY_COLOR 0x4
#define RC_TEXTURE 0x8
#define RC_SPARE0 0xC

#define MAP_UNSIGNED 0x0
#define MAP_UNSIGNED_INV 0x1
#define MAP_SIGNED 0x6
#define MAP_SIGNED_NEG 0x7

#define OP_CF RC_PRIMARY_COLOR, GL_TRUE, MAP_UNSIGNED
#define OP_AF RC_PRIMARY_COLOR, GL_FALSE, MAP_UNSIGNED
#define OP_CS RC_TEXTURE + texid, GL_TRUE, MAP_UNSIGNED
#define OP_AS RC_TEXTURE + texid, GL_FALSE, MAP_UNSIGNED
#define OP_AS_INV RC_TEXTURE + texid, GL_FALSE, MAP_UNSIGNED_INV
#define OP_CP rc_prev, GL_TRUE, MAP_UNSIGNED
#define OP_AP rc_prev, GL_FALSE, MAP_UNSIGNED
#define OP_CC0 RC_CONSTANT_COLOR0, GL_TRUE, MAP_UNSIGNED
#define OP_CA0 RC_CONSTANT_COLOR0, GL_FALSE, MAP_UNSIGNED
#define OP_CC1 RC_CONSTANT_COLOR1, GL_TRUE, MAP_UNSIGNED
#define OP_CA1 RC_CONSTANT_COLOR1, GL_FALSE, MAP_UNSIGNED
#define OP_C0 RC_ZERO, GL_TRUE, MAP_UNSIGNED
#define OP_A0 RC_ZERO, GL_FALSE, MAP_UNSIGNED
#define OP_C1 RC_ZERO, GL_TRUE, MAP_UNSIGNED_INV
#define OP_A1 RC_ZERO, GL_FALSE, MAP_UNSIGNED_INV

#define OP_COMBINE_RGB(i) \
  texenv_arg_src_gl_to_rc(texenv, texid, GL_TRUE, (i)), \
  !texenv_arg_is_alpha(texenv, GL_TRUE, (i)), \
  texenv_arg_mapping(texenv, GL_TRUE, (i))
#define OP_COMBINE_RGB_INV(i) \
  texenv_arg_src_gl_to_rc(texenv, texid, GL_TRUE, (i)), \
  !texenv_arg_is_alpha(texenv, GL_TRUE, (i)), \
  !texenv_arg_mapping(texenv, GL_TRUE, (i))
#define OP_COMBINE_A(i) \
  texenv_arg_src_gl_to_rc(texenv, texid, GL_FALSE, (i)), \
  !texenv_arg_is_alpha(texenv, GL_FALSE, (i)), \
  texenv_arg_mapping(texenv, GL_FALSE, (i))
#define OP_COMBINE_A_INV(i) \
  texenv_arg_src_gl_to_rc(texenv, texid, GL_FALSE, (i)), \
  !texenv_arg_is_alpha(texenv, GL_FALSE, (i)), \
  !texenv_arg_mapping(texenv, GL_FALSE, (i))

#define TEXENV_DIRTY_IF_CHANGED(field, value) \
  if (pbgl.texenv[pbgl.active_tex_sv].field != value) \
    pbgl.state_dirty = pbgl.texenv_dirty = GL_TRUE

static inline GLuint *texenv_push_src_rgb(
  GLuint *p,
  const GLuint stage,
  const GLuint a, const GLboolean a_rgb, const GLuint a_map,
  const GLuint b, const GLboolean b_rgb, const GLuint b_map,
  const GLuint c, const GLboolean c_rgb, const GLuint c_map,
  const GLuint d, const GLboolean d_rgb, const GLuint d_map
) {
  return pb_push1(p, NV097_SET_COMBINER_COLOR_ICW + stage * 4,
      PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_A_SOURCE, a) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_A_ALPHA, !a_rgb) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_A_MAP, a_map)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_B_SOURCE, b) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_B_ALPHA, !b_rgb) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_B_MAP, b_map)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_C_SOURCE, c) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_C_ALPHA, !c_rgb) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_C_MAP, c_map)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_D_SOURCE, d) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_D_ALPHA, !d_rgb) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_D_MAP, d_map));
}

static inline GLuint *texenv_push_src_a(
  GLuint *p,
  const GLuint stage,
  const GLuint a, const GLboolean a_rgb, const GLuint a_map,
  const GLuint b, const GLboolean b_rgb, const GLuint b_map,
  const GLuint c, const GLboolean c_rgb, const GLuint c_map,
  const GLuint d, const GLboolean d_rgb, const GLuint d_map
) {
  return pb_push1(p, NV097_SET_COMBINER_ALPHA_ICW + stage * 4,
      PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_A_SOURCE, a) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_A_ALPHA, !a_rgb) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_A_MAP, a_map)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_B_SOURCE, b) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_B_ALPHA, !b_rgb) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_B_MAP, b_map)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_C_SOURCE, c) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_C_ALPHA, !c_rgb) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_C_MAP, c_map)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_D_SOURCE, d) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_D_ALPHA, !d_rgb) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_D_MAP, d_map));
}

static inline GLuint *texenv_push_dst_rgb(GLuint *p, const GLuint stage, const GLboolean ab, const GLboolean cd, const GLboolean sum, const GLuint op) {
  return pb_push1(p, NV097_SET_COMBINER_COLOR_OCW + stage * 4,
    PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_AB_DST, ab ? RC_SPARE0 : RC_DISCARD)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_CD_DST, cd ? RC_SPARE0 : RC_DISCARD)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_SUM_DST, sum ? RC_SPARE0 : RC_DISCARD)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_MUX_ENABLE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_AB_DOT_ENABLE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_CD_DOT_ENABLE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_OP, op));
}

static inline GLuint *texenv_push_dst_a(GLuint *p, const GLuint stage, const GLboolean ab, const GLboolean cd, const GLboolean sum, const GLuint op) {
  return pb_push1(p, NV097_SET_COMBINER_ALPHA_OCW + stage * 4,
    PBGL_MASK(NV097_SET_COMBINER_ALPHA_OCW_AB_DST, ab ? RC_SPARE0 : RC_DISCARD)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_OCW_CD_DST, cd ? RC_SPARE0 : RC_DISCARD)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_OCW_SUM_DST, sum ? RC_SPARE0 : RC_DISCARD)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_OCW_MUX_ENABLE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_OCW_OP, op));
}

static inline GLuint *texenv_push_replace(GLuint *p, const GLuint texid, const GLuint rc_prev) {
  // scale only affects GL_COMBINE
  const GLuint shift_rgb = NV097_SET_COMBINER_COLOR_OCW_OP_NOSHIFT;
  const GLuint shift_a = NV097_SET_COMBINER_ALPHA_OCW_OP_NOSHIFT;
  const texture_t *tex = pbgl.tex[texid].tex;
  switch (tex->gl.baseformat == GL_COLOR_INDEX ? tex->palette.baseformat : tex->gl.baseformat) {
    case GL_ALPHA:
      // Cv = Cp
      p = texenv_push_src_rgb(p, texid, OP_CP, OP_C1, OP_C0, OP_C0);
      p = texenv_push_dst_rgb(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_rgb);
      // Av = As
      p = texenv_push_src_a(p, texid, OP_AS, OP_A1, OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    case GL_RGB:
    case GL_LUMINANCE:
      // Cv = Cs
      p = texenv_push_src_rgb(p, texid, OP_CS, OP_C1, OP_C0, OP_C0);
      p = texenv_push_dst_rgb(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_rgb);
      // Av = Ap
      p = texenv_push_src_a(p, texid, OP_AP, OP_A1, OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    case GL_RGBA:
    case GL_LUMINANCE_ALPHA:
      // Cv = Cs
      p = texenv_push_src_rgb(p, texid, OP_CS, OP_C1, OP_C0, OP_C0);
      p = texenv_push_dst_rgb(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_rgb);
      // Av = As
      p = texenv_push_src_a(p, texid, OP_AS, OP_A1, OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    default:
      break;
  }
  return p;
}

static inline GLuint *texenv_push_modulate(GLuint *p, const GLuint texid, const GLuint rc_prev) {
  // scale only affects GL_COMBINE
  const GLuint shift_rgb = NV097_SET_COMBINER_COLOR_OCW_OP_NOSHIFT;
  const GLuint shift_a = NV097_SET_COMBINER_ALPHA_OCW_OP_NOSHIFT;
  const texture_t *tex = pbgl.tex[texid].tex;
  switch (tex->gl.baseformat == GL_COLOR_INDEX ? tex->palette.baseformat : tex->gl.baseformat) {
    case GL_ALPHA:
      // Cv = Cp
      p = texenv_push_src_rgb(p, texid, OP_CP, OP_C1, OP_C0, OP_C0);
      p = texenv_push_dst_rgb(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_rgb);
      // Av = Ap * As
      p = texenv_push_src_a(p, texid, OP_AP, OP_AS, OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    case GL_RGB:
    case GL_LUMINANCE:
      // Cv = Cp * Cs
      p = texenv_push_src_rgb(p, texid, OP_CP, OP_CS, OP_C0, OP_C0);
      p = texenv_push_dst_rgb(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_rgb);
      // Av = Ap
      p = texenv_push_src_a(p, texid, OP_AP, OP_A1, OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    case GL_RGBA:
    case GL_LUMINANCE_ALPHA:
      // Cv = Cp * Cs
      p = texenv_push_src_rgb(p, texid, OP_CP, OP_CS, OP_C0, OP_C0);
      p = texenv_push_dst_rgb(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_rgb);
      // Av = Ap * As
      p = texenv_push_src_a(p, texid, OP_AP, OP_AS, OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    default:
      break;
  }
  return p;
}

static inline GLuint *texenv_push_add(GLuint *p, const GLuint texid, const GLuint rc_prev) {
  // scale only affects GL_COMBINE
  const GLuint shift_rgb = NV097_SET_COMBINER_COLOR_OCW_OP_NOSHIFT;
  const GLuint shift_a = NV097_SET_COMBINER_ALPHA_OCW_OP_NOSHIFT;
  const texture_t *tex = pbgl.tex[texid].tex;
  switch (tex->gl.baseformat == GL_COLOR_INDEX ? tex->palette.baseformat : tex->gl.baseformat) {
    case GL_ALPHA:
      // Cv = Cp
      p = texenv_push_src_rgb(p, texid, OP_CP, OP_C1, OP_C0, OP_C0);
      p = texenv_push_dst_rgb(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_rgb);
      // Av = Ap * As
      p = texenv_push_src_a(p, texid, OP_AP, OP_AS, OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    case GL_RGB:
    case GL_LUMINANCE:
      // Cv = Cp + Cs
      p = texenv_push_src_rgb(p, texid, OP_CP, OP_C1, OP_CS, OP_C1);
      p = texenv_push_dst_rgb(p, texid, GL_FALSE, GL_FALSE, GL_TRUE, shift_rgb);
      // Av = Ap
      p = texenv_push_src_a(p, texid, OP_AP, OP_A1, OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    case GL_RGBA:
    case GL_LUMINANCE_ALPHA:
      // Cv = Cp + Cs
      p = texenv_push_src_rgb(p, texid, OP_CP, OP_C1, OP_CS, OP_C1);
      p = texenv_push_dst_rgb(p, texid, GL_FALSE, GL_FALSE, GL_TRUE, shift_rgb);
      // Av = Ap * As
      p = texenv_push_src_a(p, texid, OP_AP, OP_AS, OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    default:
      break;
  }
  return p;
}

static inline GLuint *texenv_push_decal(GLuint *p, const GLuint texid, const GLuint rc_prev) {
  // scale only affects GL_COMBINE
  const GLuint shift_rgb = NV097_SET_COMBINER_COLOR_OCW_OP_NOSHIFT;
  const GLuint shift_a = NV097_SET_COMBINER_ALPHA_OCW_OP_NOSHIFT;
  const texture_t *tex = pbgl.tex[texid].tex;
  switch (tex->gl.baseformat == GL_COLOR_INDEX ? tex->palette.baseformat : tex->gl.baseformat) {
    case GL_RGB:
      // Cv = Cs
      p = texenv_push_src_rgb(p, texid, OP_CS, OP_C1, OP_C0, OP_C0);
      p = texenv_push_dst_rgb(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_rgb);
      // Av = Ap
      p = texenv_push_src_a(p, texid, OP_AP, OP_A1, OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    case GL_RGBA:
      // Cv = Cp * (1 - As) + Cs * As
      p = texenv_push_src_rgb(p, texid, OP_CP, OP_AS_INV, OP_CS, OP_AS);
      p = texenv_push_dst_rgb(p, texid, GL_FALSE, GL_FALSE, GL_TRUE, shift_rgb);
      // Av = Ap
      p = texenv_push_src_a(p, texid, OP_AP, OP_A1, OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    default:
      // this is "undefined" for the rest of the formats, so fall back to MODULATE
      return texenv_push_modulate(p, texid, rc_prev);
  }
  return p;
}

static inline GLuint texenv_arg_src_gl_to_rc(const texenv_state_t *texenv, const GLuint texid, const GLboolean rgb, const int arg) {
  const GLenum src = rgb ? texenv->src_rgb[arg] : texenv->src_a[arg];
  switch(src) {
    case GL_PREVIOUS:
      if (texid == 0) // first texture stage only has primary color before it
        return RC_PRIMARY_COLOR;
      return RC_SPARE0; // otherwise use the result of the previous stage
    case GL_PRIMARY_COLOR:
      return RC_PRIMARY_COLOR;
    case GL_TEXTURE:
      return RC_TEXTURE + texid;
    case GL_CONSTANT:
      return RC_CONSTANT_COLOR0;
    default:
      return RC_DISCARD;
  }
}

static inline GLboolean texenv_arg_is_alpha(const texenv_state_t *texenv, const GLboolean rgb, const int arg) {
  const GLenum operand = rgb ? texenv->operand_rgb[arg] : texenv->operand_a[arg];
  return (operand == GL_SRC_ALPHA || operand == GL_ONE_MINUS_SRC_ALPHA);
}

static inline GLuint texenv_arg_mapping(const texenv_state_t *texenv, const GLboolean rgb, const int arg) {
  const GLenum operand = rgb ? texenv->operand_rgb[arg] : texenv->operand_a[arg];
  return (operand == GL_ONE_MINUS_SRC_COLOR || operand == GL_ONE_MINUS_SRC_ALPHA) ? MAP_UNSIGNED_INV : MAP_UNSIGNED;
}

static inline GLuint *texenv_push_combine(GLuint *p, const texenv_state_t *texenv, const GLuint texid, const GLuint rc_prev) {
  const GLuint shift_rgb = pbgl.texenv[texid].shift_rgb;
  const GLuint shift_a = pbgl.texenv[texid].shift_a;

  switch (texenv->combine_rgb) {
    case GL_REPLACE:
      // spare0 = a = Arg0
      p = texenv_push_src_rgb(p, texid, OP_COMBINE_RGB(0), OP_C1, OP_C0, OP_C0);
      p = texenv_push_dst_rgb(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_rgb);
      break;
    case GL_MODULATE:
      // spare0 = a * b = Arg0 * Arg1
      p = texenv_push_src_rgb(p, texid, OP_COMBINE_RGB(0), OP_COMBINE_RGB(1), OP_C0, OP_C0);
      p = texenv_push_dst_rgb(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_rgb);
      break;
    case GL_INTERPOLATE:
      // spare0 = a * b + c * d = Arg0 * Arg2 + Arg1 * (1 - Arg2)
      p = texenv_push_src_rgb(p, texid, OP_COMBINE_RGB(0), OP_COMBINE_RGB(2), OP_COMBINE_RGB(1), OP_COMBINE_RGB_INV(2));
      p = texenv_push_dst_rgb(p, texid, GL_FALSE, GL_FALSE, GL_TRUE, shift_rgb);
      break;
    case GL_ADD:
      // spare0 = a * b + c * d = a * 1 + c * 1 = Arg0 + Arg1
      p = texenv_push_src_rgb(p, texid, OP_COMBINE_RGB(0), OP_C1, OP_COMBINE_RGB(1), OP_C1);
      p = texenv_push_dst_rgb(p, texid, GL_FALSE, GL_FALSE, GL_TRUE, shift_rgb);
      break;
    default:
      // TODO: GL_ADD_SIGNED, GL_SUBTRACT, GL_DOT3*
      break;
  }

  switch (texenv->combine_a) {
    case GL_REPLACE:
      // spare0 = a = Arg0
      p = texenv_push_src_a(p, texid, OP_COMBINE_A(0), OP_A1, OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    case GL_MODULATE:
      // spare0 = a * b = Arg0 * Arg1
      p = texenv_push_src_a(p, texid, OP_COMBINE_A(0), OP_COMBINE_A(1), OP_A0, OP_A0);
      p = texenv_push_dst_a(p, texid, GL_TRUE, GL_FALSE, GL_FALSE, shift_a);
      break;
    case GL_INTERPOLATE:
      // spare0 = a * b + c * d = Arg0 * Arg2 + Arg1 * (1 - Arg2)
      p = texenv_push_src_a(p, texid, OP_COMBINE_A(0), OP_COMBINE_A(2), OP_COMBINE_A(1), OP_COMBINE_A_INV(2));
      p = texenv_push_dst_a(p, texid, GL_FALSE, GL_FALSE, GL_TRUE, shift_a);
      break;
    case GL_ADD:
      // spare0 = a * b + c * d = a * 1 + c * 1 = Arg0 + Arg1
      p = texenv_push_src_a(p, texid, OP_COMBINE_A(0), OP_A1, OP_COMBINE_A(1), OP_A1);
      p = texenv_push_dst_a(p, texid, GL_FALSE, GL_FALSE, GL_TRUE, shift_a);
      break;
    default:
      // TODO: GL_ADD_SIGNED, GL_SUBTRACT, GL_DOT3*
      break;
  }

  return p;
}

static inline GLboolean texenv_mode_valid(const GLenum mode) {
  switch (mode) {
    case GL_REPLACE:
    case GL_MODULATE:
    case GL_DECAL:
    case GL_COMBINE:
    case GL_ADD:
      return GL_TRUE;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      return GL_FALSE;
  }
}

static inline GLboolean texenv_combine_mode_valid(const GLenum mode) {
  switch (mode) {
    case GL_REPLACE:
    case GL_MODULATE:
    case GL_INTERPOLATE:
    case GL_ADD:
      return GL_TRUE;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      return GL_FALSE;
  }
}

/* pbgl internals */

GLuint *pbgl_texenv_push(GLuint *p) {
  GLuint num_stages = 0;
  GLuint rc_prev = RC_PRIMARY_COLOR;
  GLuint shadermask[TEXUNIT_COUNT] = { 0 };

  for (GLuint i = 0; i < TEXUNIT_COUNT; ++i) {
    texenv_state_t *texenv = pbgl.texenv + i;

    // set constant color if it has changed
    if (texenv->color_dirty) {
      p = pb_push1(p, NV097_SET_COMBINER_FACTOR0, texenv->color);
      texenv->color_dirty = GL_FALSE;
    }

    // TODO: properly handle texture_1d, texture_3d, texture_cube
    const GLboolean tex_enabled =
      pbgl.tex[i].flags.texture_1d ||
      pbgl.tex[i].flags.texture_2d;
    if (tex_enabled && pbgl.tex[i].enabled && pbgl.tex[i].tex) {
      switch (texenv->mode) {
        case GL_REPLACE:
          p = texenv_push_replace(p, i, rc_prev);
          break;
        case GL_MODULATE:
          p = texenv_push_modulate(p, i, rc_prev);
          break;
        case GL_DECAL:
          p = texenv_push_decal(p, i, rc_prev);
          break;
        case GL_ADD:
          p = texenv_push_add(p, i, rc_prev);
          break;
        case GL_COMBINE:
          p = texenv_push_combine(p, texenv, i, rc_prev);
          break;
        default:
          // TODO: GL_BLEND, GL_INTERPOLATE
          break;
      }

      // enable 2D projective mode for this stage
      shadermask[i] = NV097_SET_SHADER_STAGE_PROGRAM_STAGE0_2D_PROJECTIVE;

      // remember the last active stage
      num_stages = i + 1;

      // use the output of the previous active stage next
      rc_prev = RC_SPARE0;
    }
  }

  // set up passthrough stage for disabled texunits
  for (GLuint i = 0; i < TEXUNIT_COUNT; ++i) {
    if (!shadermask[i]) {
      p = pb_push1(p, NV097_SET_COMBINER_COLOR_ICW + i * 4, 0);
      p = pb_push1(p, NV097_SET_COMBINER_COLOR_OCW + i * 4, 0);
      p = pb_push1(p, NV097_SET_COMBINER_ALPHA_ICW + i * 4, 0);
      p = pb_push1(p, NV097_SET_COMBINER_ALPHA_OCW + i * 4, 0);
    }
  }

  const GLuint reg_out = rc_prev;
  if (num_stages == 0) {
    // textures are disabled, use the primary color register for output
    num_stages++;
  }

  // push shader modes
  p = pb_push1(p, NV097_SET_SHADER_STAGE_PROGRAM,
      PBGL_MASK(NV097_SET_SHADER_STAGE_PROGRAM_STAGE0, shadermask[0])
    | PBGL_MASK(NV097_SET_SHADER_STAGE_PROGRAM_STAGE1, shadermask[1])
    | PBGL_MASK(NV097_SET_SHADER_STAGE_PROGRAM_STAGE2, shadermask[2])
    | PBGL_MASK(NV097_SET_SHADER_STAGE_PROGRAM_STAGE3, shadermask[3]));

  // set up final combiner
  // this should be `out.rgb = spare0.rgb; out.a = spare0.a;`
  p = pb_push1(p, NV097_SET_COMBINER_CONTROL,
      PBGL_MASK(NV097_SET_COMBINER_CONTROL_FACTOR0, NV097_SET_COMBINER_CONTROL_FACTOR0_SAME_FACTOR_ALL)
    | PBGL_MASK(NV097_SET_COMBINER_CONTROL_FACTOR1, NV097_SET_COMBINER_CONTROL_FACTOR1_SAME_FACTOR_ALL)
    | PBGL_MASK(NV097_SET_COMBINER_CONTROL_ITERATION_COUNT, num_stages));
  p = pb_push1(p, NV097_SET_COMBINER_SPECULAR_FOG_CW0,
      PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_A_SOURCE, RC_ZERO) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_A_ALPHA, 0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_A_INVERSE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_B_SOURCE, RC_ZERO) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_B_ALPHA, 0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_B_INVERSE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_C_SOURCE, RC_ZERO) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_C_ALPHA, 0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_C_INVERSE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_D_SOURCE, reg_out) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_D_ALPHA, 0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_D_INVERSE, 0));
  p = pb_push1(p, NV097_SET_COMBINER_SPECULAR_FOG_CW1,
      PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_E_SOURCE, RC_ZERO) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_E_ALPHA, 0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_E_INVERSE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_F_SOURCE, RC_ZERO) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_F_ALPHA, 0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_F_INVERSE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_G_SOURCE, reg_out) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_G_ALPHA, 1) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_G_INVERSE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_SPECULAR_CLAMP, 0));

  return p;
}

/* GL FUNCTIONS BEGIN */

GL_API void glTexEnvi(GLenum target, GLenum pname, GLint param) {
  if (target != GL_TEXTURE_ENV) {
    // TODO: support others
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  GLuint idx;
  switch (pname) {
    case GL_TEXTURE_ENV_MODE:
      if (texenv_mode_valid(param)) {
        TEXENV_DIRTY_IF_CHANGED(mode, param);
        pbgl.texenv[pbgl.active_tex_sv].mode = param;
      }
      break;

    case GL_COMBINE_RGB:
      if (texenv_combine_mode_valid(param)) {
        TEXENV_DIRTY_IF_CHANGED(combine_rgb, param);
        pbgl.texenv[pbgl.active_tex_sv].combine_rgb = param;
      }
      break;
    case GL_COMBINE_ALPHA:
      if (texenv_combine_mode_valid(param)) {
        TEXENV_DIRTY_IF_CHANGED(combine_a, param);
        pbgl.texenv[pbgl.active_tex_sv].combine_a = param;
      }
      break;

    case GL_SRC0_RGB:
    case GL_SRC1_RGB:
    case GL_SRC2_RGB:
      idx = pname - GL_SRC0_RGB;
      TEXENV_DIRTY_IF_CHANGED(src_rgb[idx], param);
      pbgl.texenv[pbgl.active_tex_sv].src_rgb[idx] = param;
      break;

    case GL_SRC0_ALPHA:
    case GL_SRC1_ALPHA:
    case GL_SRC2_ALPHA:
      idx = pname - GL_SRC0_ALPHA;
      TEXENV_DIRTY_IF_CHANGED(src_a[idx], param);
      pbgl.texenv[pbgl.active_tex_sv].src_a[idx] = param;
      break;

    case GL_OPERAND0_RGB:
    case GL_OPERAND1_RGB:
    case GL_OPERAND2_RGB:
      idx = pname - GL_OPERAND0_RGB;
      TEXENV_DIRTY_IF_CHANGED(operand_rgb[idx], param);
      pbgl.texenv[pbgl.active_tex_sv].operand_rgb[idx] = param;
      break;

    case GL_OPERAND0_ALPHA:
    case GL_OPERAND1_ALPHA:
    case GL_OPERAND2_ALPHA:
      idx = pname - GL_OPERAND0_ALPHA;
      TEXENV_DIRTY_IF_CHANGED(operand_a[idx], param);
      pbgl.texenv[pbgl.active_tex_sv].operand_a[idx] = param;
      break;

    case GL_RGB_SCALE:
      if (param != 1 && param != 2 && param != 4) {
        pbgl_set_error(GL_INVALID_VALUE);
        break;
      }
      // param already directly maps to NV097_SET_COMBINER_COLOR_OCW_OP_SHIFTLEFTBYx,
      // except 1, which would map to NOSHIFT_BIAS instead of NOSHIFT
      if (param == 1) param = 0;
      TEXENV_DIRTY_IF_CHANGED(shift_rgb, param);
      pbgl.texenv[pbgl.active_tex_sv].shift_rgb = param;
      break;

    case GL_ALPHA_SCALE:
      if (param != 1 && param != 2 && param != 4) {
        pbgl_set_error(GL_INVALID_VALUE);
        break;
      }
      // param already directly maps to NV097_SET_COMBINER_ALPHA_OCW_OP_SHIFTLEFTBYx
      // except 1, which would map to NOSHIFT_BIAS instead of NOSHIFT
      if (param == 1) param = 0;
      TEXENV_DIRTY_IF_CHANGED(shift_a, param);
      pbgl.texenv[pbgl.active_tex_sv].shift_a = param;
      break;

    default:
      pbgl_set_error(GL_INVALID_ENUM);
      break;
  }  
}

GL_API void glTexEnvf(GLenum target, GLenum pname, GLfloat param) {
  if (target != GL_TEXTURE_ENV) {
    // TODO: support others
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  switch (pname) {
    default:
      glTexEnvi(target, pname, (GLint)param);
      break;
  }
}

GL_API void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *param) {
  if (target != GL_TEXTURE_ENV) {
    // TODO: support others
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  if (!param) {
    pbgl_set_error(GL_INVALID_VALUE);
    return;
  }

  switch (pname) {
    case GL_TEXTURE_ENV_COLOR: {
      const GLuint r = (param[0] * 255.f);
      const GLuint g = (param[1] * 255.f);
      const GLuint b = (param[2] * 255.f);
      const GLuint a = (param[3] * 255.f);
      const GLuint rgba = (a << 24) | (r << 16) | (g << 8) | b;
      if (pbgl.texenv[pbgl.active_tex_sv].color != rgba) {
        pbgl.state_dirty = pbgl.texenv_dirty = GL_TRUE;
        pbgl.texenv[pbgl.active_tex_sv].color_dirty = GL_TRUE;
        pbgl.texenv[pbgl.active_tex_sv].color = rgba;
      }
      break;
    }
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      break;
  }
}

GL_API void glTexEnviv(GLenum target, GLenum pname, const GLint *param) {
  if (target != GL_TEXTURE_ENV) {
    // TODO: support others
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  if (!param) {
    pbgl_set_error(GL_INVALID_VALUE);
    return;
  }

  switch (pname) {
    case GL_TEXTURE_ENV_COLOR: {
      // "Integer color components are interpreted linearly such that
      // the most positive integer maps to 1.0, and the most negative integer maps to -1.0""
      // what the fuck did they mean by this?
      const GLuint r = ((GLdouble)param[0] / (1.0 + (GLdouble)INT_MAX) * 255.0);
      const GLuint g = ((GLdouble)param[1] / (1.0 + (GLdouble)INT_MAX) * 255.0);
      const GLuint b = ((GLdouble)param[2] / (1.0 + (GLdouble)INT_MAX) * 255.0);
      const GLuint a = ((GLdouble)param[3] / (1.0 + (GLdouble)INT_MAX) * 255.0);
      const GLuint rgba = (a << 24) | (r << 16) | (g << 8) | b;
      if (pbgl.texenv[pbgl.active_tex_sv].color != rgba) {
        pbgl.state_dirty = pbgl.texenv_dirty = GL_TRUE;
        pbgl.texenv[pbgl.active_tex_sv].color_dirty = GL_TRUE;
        pbgl.texenv[pbgl.active_tex_sv].color = rgba;
      }
      break;
    }
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      break;
  }
}
