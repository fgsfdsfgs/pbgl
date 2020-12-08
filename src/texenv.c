#include <pbkit/pbkit.h>

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
#define RC_PRIMARY_COLOR 0x4
#define RC_TEXTURE 0x8
#define RC_SPARE0 0xC

#define RC_UNSIGNED 0x0
#define RC_UNSIGNED_INVERT 0x1

#define OP_CF RC_PRIMARY_COLOR, GL_TRUE, GL_FALSE
#define OP_AF RC_PRIMARY_COLOR, GL_FALSE, GL_FALSE
#define OP_CS RC_TEXTURE + texid, GL_TRUE, GL_FALSE
#define OP_AS RC_TEXTURE + texid, GL_FALSE, GL_FALSE
#define OP_CP rc_prev, GL_TRUE, GL_FALSE
#define OP_AP rc_prev, GL_FALSE, GL_FALSE
#define OP_ZERO RC_ZERO, GL_TRUE, GL_FALSE
#define OP_ONE RC_ZERO, GL_TRUE, GL_TRUE
#define OP_AS_INV RC_TEXTURE + texid, GL_FALSE, GL_TRUE

#define OP_COMBINE_RGB(i) \
  texenv_arg_src_gl_to_rc(texenv, texid, GL_TRUE, (i)), \
  texenv_arg_is_alpha(texenv, GL_TRUE, (i)), \
  texenv_arg_is_inverted(texenv, GL_TRUE, (i))
#define OP_COMBINE_RGB_INV(i) \
  texenv_arg_src_gl_to_rc(texenv, texid, GL_TRUE, (i)), \
  texenv_arg_is_alpha(texenv, GL_TRUE, (i)), \
  !texenv_arg_is_inverted(texenv, GL_TRUE, (i))
#define OP_COMBINE_A(i) \
  texenv_arg_src_gl_to_rc(texenv, texid, GL_FALSE, (i)), \
  texenv_arg_is_alpha(texenv, GL_FALSE, (i)), \
  texenv_arg_is_inverted(texenv, GL_FALSE, (i))
#define OP_COMBINE_A_INV(i) \
  texenv_arg_src_gl_to_rc(texenv, texid, GL_FALSE, (i)), \
  texenv_arg_is_alpha(texenv, GL_FALSE, (i)), \
  !texenv_arg_is_inverted(texenv, GL_FALSE, (i))

#define TEXENV_DIRTY_IF_CHANGED(field, value) \
  if (pbgl.texenv[pbgl.active_tex_sv].field != value) \
    pbgl.state_dirty = pbgl.texenv_dirty = GL_TRUE

static inline GLuint *texenv_push_src_rgb(
  GLuint *p,
  const GLuint stage,
  const GLuint a, const GLboolean a_rgb, const GLboolean a_inv,
  const GLuint b, const GLboolean b_rgb, const GLboolean b_inv,
  const GLuint c, const GLboolean c_rgb, const GLboolean c_inv,
  const GLuint d, const GLboolean d_rgb, const GLboolean d_inv
) {
  return pb_push1(p, NV097_SET_COMBINER_COLOR_ICW + stage * 4,
      PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_A_SOURCE, a) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_A_ALPHA, !a_rgb) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_A_MAP, a_inv)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_B_SOURCE, b) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_B_ALPHA, !b_rgb) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_B_MAP, b_inv)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_C_SOURCE, c) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_C_ALPHA, !c_rgb) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_C_MAP, c_inv)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_D_SOURCE, d) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_D_ALPHA, !d_rgb) | PBGL_MASK(NV097_SET_COMBINER_COLOR_ICW_D_MAP, d_inv));
}

static inline GLuint *texenv_push_src_a(
  GLuint *p,
  const GLuint stage,
  const GLuint a, const GLboolean a_rgb, const GLboolean a_inv,
  const GLuint b, const GLboolean b_rgb, const GLboolean b_inv,
  const GLuint c, const GLboolean c_rgb, const GLboolean c_inv,
  const GLuint d, const GLboolean d_rgb, const GLboolean d_inv
) {
  return pb_push1(p, NV097_SET_COMBINER_ALPHA_ICW + stage * 4,
      PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_A_SOURCE, a) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_A_ALPHA, !a_rgb) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_A_MAP, a_inv)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_B_SOURCE, b) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_B_ALPHA, !b_rgb) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_B_MAP, b_inv)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_C_SOURCE, c) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_C_ALPHA, !c_rgb) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_C_MAP, c_inv)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_D_SOURCE, d) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_D_ALPHA, !d_rgb) | PBGL_MASK(NV097_SET_COMBINER_ALPHA_ICW_D_MAP, d_inv));
}

static inline GLuint *texenv_push_dst_rgb(GLuint *p, const GLuint stage, const GLboolean ab, const GLboolean cd, const GLboolean sum) {
  return pb_push1(p, NV097_SET_COMBINER_COLOR_OCW + stage * 4,
    PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_AB_DST, ab ? RC_SPARE0 : RC_DISCARD)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_CD_DST, cd ? RC_SPARE0 : RC_DISCARD)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_SUM_DST, sum ? RC_SPARE0 : RC_DISCARD)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_MUX_ENABLE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_AB_DOT_ENABLE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_CD_DOT_ENABLE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_COLOR_OCW_OP, NV097_SET_COMBINER_COLOR_OCW_OP_NOSHIFT));
}

static inline GLuint *texenv_push_dst_a(GLuint *p, const GLuint stage, const GLboolean ab, const GLboolean cd, const GLboolean sum) {
  return pb_push1(p, NV097_SET_COMBINER_ALPHA_OCW + stage * 4,
    PBGL_MASK(NV097_SET_COMBINER_ALPHA_OCW_AB_DST, ab ? RC_SPARE0 : RC_DISCARD)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_OCW_CD_DST, cd ? RC_SPARE0 : RC_DISCARD)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_OCW_SUM_DST, sum ? RC_SPARE0 : RC_DISCARD)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_OCW_MUX_ENABLE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_ALPHA_OCW_OP, NV097_SET_COMBINER_ALPHA_OCW_OP_NOSHIFT));
}

static inline GLuint *texenv_push_replace(GLuint *p, const GLuint texid, const GLuint stage, const GLuint rc_prev) {
  const texture_t *tex = pbgl.tex[texid].tex;
  switch (tex->gl.baseformat) {
    case GL_ALPHA:
      // Cv = Cp
      p = texenv_push_src_rgb(p, stage, OP_CP, OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_rgb(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      // Av = As
      p = texenv_push_src_a(p, stage, OP_AS, OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_a(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      break;
    case GL_RGB:
    case GL_LUMINANCE:
      // Cv = Cs
      p = texenv_push_src_rgb(p, stage, OP_CS, OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_rgb(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      // Av = Ap
      p = texenv_push_src_a(p, stage, OP_AP, OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_a(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      break;
    case GL_RGBA:
    case GL_LUMINANCE_ALPHA:
      // Cv = Cs
      p = texenv_push_src_rgb(p, stage, OP_CS, OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_rgb(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      // Av = As
      p = texenv_push_src_a(p, stage, OP_AS, OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_a(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      break;
    default:
      break;
  }
  return p;
}

static inline GLuint *texenv_push_modulate(GLuint *p, const GLuint texid, const GLuint stage, const GLuint rc_prev) {
  const texture_t *tex = pbgl.tex[texid].tex;
  switch (tex->gl.baseformat) {
    case GL_ALPHA:
      // Cv = Cp
      p = texenv_push_src_rgb(p, stage, OP_CP, OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_rgb(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      // Av = Ap * As
      p = texenv_push_src_a(p, stage, OP_AP, OP_AS, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_a(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      break;
    case GL_RGB:
    case GL_LUMINANCE:
      // Cv = Cp * Cs
      p = texenv_push_src_rgb(p, stage, OP_CP, OP_CS, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_rgb(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      // Av = Ap
      p = texenv_push_src_a(p, stage, OP_AP, OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_a(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      break;
    case GL_RGBA:
    case GL_LUMINANCE_ALPHA:
      // Cv = Cp * Cs
      p = texenv_push_src_rgb(p, stage, OP_CP, OP_CS, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_rgb(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      // Av = Ap * As
      p = texenv_push_src_a(p, stage, OP_AP, OP_AS, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_a(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      break;
    default:
      break;
  }
  return p;
}

static inline GLuint *texenv_push_decal(GLuint *p, const GLuint texid, const GLuint stage, const GLuint rc_prev) {
  const texture_t *tex = pbgl.tex[texid].tex;
  switch (tex->gl.baseformat) {
    case GL_RGB:
      // Cv = Cs
      p = texenv_push_src_rgb(p, stage, OP_CS, OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_rgb(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      // Av = Ap
      p = texenv_push_src_a(p, stage, OP_AP, OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_a(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      break;
    case GL_RGBA:
      // Cv = Cp * (1 - As) + Cs * As
      p = texenv_push_src_rgb(p, stage, OP_CP, OP_AS_INV, OP_CS, OP_AS);
      p = texenv_push_dst_rgb(p, stage, GL_FALSE, GL_FALSE, GL_TRUE);
      // Av = Ap
      p = texenv_push_src_a(p, stage, OP_AP, OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_a(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      break;
    default:
      // this is "undefined" for the rest of the formats
      break;
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
    default:
      return 0;
  }
}

static inline GLboolean texenv_arg_is_alpha(const texenv_state_t *texenv, const GLboolean rgb, const int arg) {
  const GLenum operand = rgb ? texenv->operand_rgb[arg] : texenv->operand_a[arg];
  return (operand == GL_SRC_ALPHA || operand == GL_ONE_MINUS_SRC_ALPHA);
}

static inline GLboolean texenv_arg_is_inverted(const texenv_state_t *texenv, const GLboolean rgb, const int arg) {
  const GLenum operand = rgb ? texenv->operand_rgb[arg] : texenv->operand_a[arg];
  return (operand == GL_ONE_MINUS_SRC_COLOR || operand == GL_ONE_MINUS_SRC_ALPHA);
}

static inline GLuint *texenv_push_combine(GLuint *p, const texenv_state_t *texenv, const GLuint texid, const GLuint stage, const GLuint rc_prev) {
  const texture_t *tex = pbgl.tex[texid].tex;

  switch (texenv->combine_rgb) {
    case GL_REPLACE:
      // spare0 = a = Arg0
      p = texenv_push_src_rgb(p, stage, OP_COMBINE_RGB(0), OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_rgb(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      break;
    case GL_MODULATE:
      // spare0 = a * b = Arg0 * Arg1
      p = texenv_push_src_rgb(p, stage, OP_COMBINE_RGB(0), OP_COMBINE_RGB(1), OP_ZERO, OP_ZERO);
      p = texenv_push_dst_rgb(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      break;
    case GL_INTERPOLATE:
      // spare0 = a * b + c * d = Arg0 * Arg2 + Arg1 * (1 - Arg2)
      p = texenv_push_src_rgb(p, stage, OP_COMBINE_RGB(0), OP_COMBINE_RGB(2), OP_COMBINE_RGB(1), OP_COMBINE_RGB_INV(2));
      p = texenv_push_dst_rgb(p, stage, GL_FALSE, GL_FALSE, GL_TRUE);
      break;
    case GL_ADD:
      // spare0 = a * b + c * d = a * 1 + c * 1 = Arg0 + Arg1
      p = texenv_push_src_rgb(p, stage, OP_COMBINE_RGB(0), OP_ONE, OP_COMBINE_RGB(1), OP_ONE);
      p = texenv_push_dst_rgb(p, stage, GL_FALSE, GL_FALSE, GL_TRUE);
      break;
    default:
      // TODO: GL_ADD_SIGNED, GL_SUBTRACT, GL_DOT3*
      break;
  }

  switch (texenv->combine_a) {
    case GL_REPLACE:
      // spare0 = a = Arg0
      p = texenv_push_src_a(p, stage, OP_COMBINE_A(0), OP_ONE, OP_ZERO, OP_ZERO);
      p = texenv_push_dst_a(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      break;
    case GL_MODULATE:
      // spare0 = a * b = Arg0 * Arg1
      p = texenv_push_src_a(p, stage, OP_COMBINE_A(0), OP_COMBINE_A(1), OP_ZERO, OP_ZERO);
      p = texenv_push_dst_a(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
      break;
    case GL_INTERPOLATE:
      // spare0 = a * b + c * d = Arg0 * Arg2 + Arg1 * (1 - Arg2)
      p = texenv_push_src_a(p, stage, OP_COMBINE_A(0), OP_COMBINE_A(2), OP_COMBINE_A(1), OP_COMBINE_A_INV(2));
      p = texenv_push_dst_a(p, stage, GL_FALSE, GL_FALSE, GL_TRUE);
      break;
    case GL_ADD:
      // spare0 = a * b + c * d = a * 1 + c * 1 = Arg0 + Arg1
      p = texenv_push_src_a(p, stage, OP_COMBINE_A(0), OP_ONE, OP_COMBINE_A(1), OP_ONE);
      p = texenv_push_dst_a(p, stage, GL_FALSE, GL_FALSE, GL_TRUE);
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
  GLuint stage = 0;
  GLuint rc_prev = RC_PRIMARY_COLOR;

  if (pbgl.flags.texture_1d || pbgl.flags.texture_2d) {
    for (GLuint i = 0; i < TEXUNIT_COUNT; ++i) {
      texenv_state_t *texenv = pbgl.texenv + i;
      if (pbgl.tex[i].enabled && pbgl.tex[i].tex) {
        switch (texenv->mode) {
          case GL_REPLACE:
            p = texenv_push_replace(p, i, stage, rc_prev);
            break;
          case GL_MODULATE:
            p = texenv_push_modulate(p, i, stage, rc_prev);
            break;
          case GL_DECAL:
            p = texenv_push_decal(p, i, stage, rc_prev);
            break;
          case GL_COMBINE:
            p = texenv_push_combine(p, texenv, i, stage, rc_prev);
            break;
          default:
            // TODO: GL_BLEND, GL_INTERPOLATE, GL_ADD
            break;
        }

        // use the output of the previous stage next
        rc_prev = RC_SPARE0;

        // make sure texture is used at this stage
        texenv->nvshader = NV097_SET_SHADER_STAGE_PROGRAM_STAGE0_2D_PROJECTIVE;

        ++stage;
      } else {
        // don't use texture color on this stage if it's disabled
        texenv->nvshader = NV097_SET_SHADER_STAGE_PROGRAM_STAGE0_PROGRAM_NONE;
      }
    }
  }

  if (stage == 0) {
    // textures are disabled, insert default fragment color stage
    // Cv = Cf
    p = texenv_push_src_rgb(p, stage, OP_CF, OP_ONE, OP_ZERO, OP_ZERO);
    p = texenv_push_dst_rgb(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
    // Av = Af
    p = texenv_push_src_a(p, stage, OP_AF, OP_ONE, OP_ZERO, OP_ZERO);
    p = texenv_push_dst_a(p, stage, GL_TRUE, GL_FALSE, GL_FALSE);
    ++stage;
  }

  // push shader modes
  p = pb_push1(p, NV097_SET_SHADER_STAGE_PROGRAM,
      PBGL_MASK(NV097_SET_SHADER_STAGE_PROGRAM_STAGE0, pbgl.texenv[0].nvshader)
    | PBGL_MASK(NV097_SET_SHADER_STAGE_PROGRAM_STAGE1, pbgl.texenv[1].nvshader)
    | PBGL_MASK(NV097_SET_SHADER_STAGE_PROGRAM_STAGE2, pbgl.texenv[2].nvshader)
    | PBGL_MASK(NV097_SET_SHADER_STAGE_PROGRAM_STAGE3, pbgl.texenv[3].nvshader));

  // set up final combiner
  // this should be `out.rgb = spare0.rgb; out.a = spare0.a;`
  p = pb_push1(p, NV097_SET_COMBINER_CONTROL,
      PBGL_MASK(NV097_SET_COMBINER_CONTROL_FACTOR0, NV097_SET_COMBINER_CONTROL_FACTOR0_SAME_FACTOR_ALL)
    | PBGL_MASK(NV097_SET_COMBINER_CONTROL_FACTOR1, NV097_SET_COMBINER_CONTROL_FACTOR1_SAME_FACTOR_ALL)
    | PBGL_MASK(NV097_SET_COMBINER_CONTROL_ITERATION_COUNT, stage));
  p = pb_push1(p, NV097_SET_COMBINER_SPECULAR_FOG_CW0,
      PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_A_SOURCE, RC_ZERO)   | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_A_ALPHA, 0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_A_INVERSE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_B_SOURCE, RC_ZERO)   | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_B_ALPHA, 0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_B_INVERSE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_C_SOURCE, RC_ZERO)   | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_C_ALPHA, 0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_C_INVERSE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_D_SOURCE, RC_SPARE0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_D_ALPHA, 0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW0_D_INVERSE, 0));
  p = pb_push1(p, NV097_SET_COMBINER_SPECULAR_FOG_CW1,
      PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_E_SOURCE, RC_ZERO)   | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_E_ALPHA, 0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_E_INVERSE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_F_SOURCE, RC_ZERO)   | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_F_ALPHA, 0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_F_INVERSE, 0)
    | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_G_SOURCE, RC_SPARE0) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_G_ALPHA, 1) | PBGL_MASK(NV097_SET_COMBINER_SPECULAR_FOG_CW1_G_INVERSE, 0)
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

    default:
      pbgl_set_error(GL_INVALID_ENUM);
      break;
  }  
}

GL_API void glTexEnvf(GLenum target, GLenum pname, GLfloat param) {
  // TODO
  glTexEnvi(target, pname, (GLint)param);
}
