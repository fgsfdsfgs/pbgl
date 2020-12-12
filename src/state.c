#include <string.h>
#include <pbkit/pbkit.h>

#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "matrix.h"
#include "texture.h"
#include "array.h"
#include "error.h"
#include "light.h"
#include "texenv.h"
#include "push.h"
#include "misc.h"
#include "state.h"

// TODO: what do the bits mean?
#define NV_TEX_DISABLE   0x0003FFC0
#define NV_TEX_ENABLE    0x40000000
#define NV_TEX_ALPHAKILL (1 << 2)

#define FLAG_DIRTY_IF_CHANGED(state, flag, value) \
  if (pbgl.flags.flag != value) \
    pbgl.state_dirty = pbgl.state.dirty = GL_TRUE

gl_state_t pbgl;

void pbgl_state_init(void) {
  memset(&pbgl, 0, sizeof(pbgl));

  /* set default state */

  pbgl.fb_width = pb_back_buffer_width();
  pbgl.fb_height = pb_back_buffer_height();

  pbgl.flags.dither = GL_TRUE;
  pbgl.error = GL_NO_ERROR;

  // nv2a compare func index = GL func index - 0x200
  pbgl.alpha.func = GL_ALWAYS & 0xF;
  pbgl.alpha.dirty = GL_TRUE;

  // src and dst blend factors are equal between nv2a and GL
  pbgl.blend.src = GL_ONE;
  pbgl.blend.dst = GL_ZERO;
  pbgl.blend.eqn = GL_FUNC_ADD; // same goes for the equation indices
  pbgl.blend.color = 0x00000000;
  pbgl.blend.dirty = GL_TRUE;

  pbgl.depth.func = GL_LESS & 0xF;
  pbgl.depth.writemask = GL_TRUE;
  pbgl.depth.dirty = GL_TRUE;

  pbgl.stencil.func = GL_ALWAYS & 0xF;
  pbgl.stencil.op_sfail = GL_KEEP; // and for stencil ops
  pbgl.stencil.op_zfail = GL_KEEP;
  pbgl.stencil.op_zpass = GL_KEEP;
  pbgl.stencil.funcmask = 0xFFFFFFFF;
  pbgl.stencil.writemask = 0xFFFFFFFF;
  pbgl.stencil.dirty = GL_TRUE;

  pbgl.cullface.frontface = GL_CCW; // and for front face
  pbgl.cullface.cullface = GL_BACK; // and for cull face
  pbgl.cullface.dirty = GL_TRUE;

  pbgl.polyofs.dirty = GL_TRUE;

  pbgl.colormask.value = 
      NV097_SET_COLOR_MASK_RED_WRITE_ENABLE
    | NV097_SET_COLOR_MASK_GREEN_WRITE_ENABLE
    | NV097_SET_COLOR_MASK_BLUE_WRITE_ENABLE
    | NV097_SET_COLOR_MASK_ALPHA_WRITE_ENABLE;
  pbgl.colormask.dirty = GL_TRUE;

  for (int i = 1; i < LIGHT_COUNT; ++i) {
    pbgl.light[i].ambient = (vec4f){{ 0.f, 0.f, 0.f, 1.f }};
    pbgl.light[i].diffuse = (vec4f){{ 0.f, 0.f, 0.f, 1.f }};
    pbgl.light[i].specular = (vec4f){{ 0.f, 0.f, 0.f, 1.f }};
    pbgl.light[i].pos = (vec4f){{ 0.f, 0.f, 1.f, 0.f }};
    pbgl.light[i].factor[0] = 1.f; // GL_CONSTANT_ATTENUATION
    pbgl.light[i].dirty = GL_TRUE;
  }
  // light #0 has a different initial state for some reason
  pbgl.light[0].ambient = (vec4f){{ 1.f, 1.f, 1.f, 1.f }};
  pbgl.light[0].diffuse = (vec4f){{ 1.f, 1.f, 1.f, 1.f }};
  pbgl.light[0].specular = (vec4f){{ 1.f, 1.f, 1.f, 1.f }};
  pbgl.light[0].pos = (vec4f){{ 0.f, 0.f, 1.f, 0.f }};
  pbgl.light[0].factor[0] = 1.f; // GL_CONSTANT_ATTENUATION
  pbgl.light[0].dirty = GL_TRUE;
  pbgl.light_any_dirty = GL_TRUE;

  pbgl.lightmodel.ambient = (vec4f){{ 0.2f, 0.2f, 0.2f, 1.0f }};
  pbgl.lightmodel.dirty = GL_TRUE;

  pbgl.imm.color    = (vec4f) {{ 1.f, 1.f, 1.f, 1.f }};
  pbgl.imm.normal   = (vec3f) {{ 0.f, 0.f, 1.f }};
  pbgl.imm.texcoord = (vec4f) {{ 0.f, 0.f, 0.f, 1.f }};

  for (int i = 0; i < TEXUNIT_COUNT; ++i) {
    pbgl.tex[i].dirty = GL_TRUE;
    pbgl.texenv[i] = (texenv_state_t) {
      .mode        = GL_MODULATE,
      .combine_rgb = GL_MODULATE,
      .combine_a   = GL_MODULATE,
      .src_rgb     = { GL_TEXTURE, GL_PREVIOUS, GL_CONSTANT },
      .src_a       = { GL_TEXTURE, GL_PREVIOUS, GL_CONSTANT },
      .operand_rgb = { GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA },
      .operand_a   = { GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA },
      .color       = 0x00000000,
      .scale_rgb   = 1.f,
      .scale_a     = 1.f,
      .dirty       = GL_TRUE,
    };
  }
  pbgl.tex_any_dirty = GL_TRUE;
  pbgl.texenv_dirty = GL_TRUE;

  for (int i = 0; i < MTX_COUNT; ++i)
    pbgl_mtx_reset(i);
  pbgl.mtx_any_dirty = GL_TRUE;
  pbgl.mtx_current = MTX_MODELVIEW;

  pbgl.view.x = 0;
  pbgl.view.y = 0;
  pbgl.view.w = pbgl.fb_width;
  pbgl.view.h = pbgl.fb_height;
  pbgl.view.znear = 0.f; // this is multiplied by appropriate Z scales below
  pbgl.view.zfar = 1.f;
  pbgl.view.dirty = GL_TRUE;

  // TODO:
  pbgl.scissor.dirty = GL_TRUE;

  pbgl.state_dirty = GL_TRUE;

  pbgl.active = GL_TRUE; // this can only be used when we're live
}

static inline GLuint *flush_texunit(GLuint *p, GLuint i) {
  texunit_state_t *tu = pbgl.tex + i;

  if (tu->dirty) {
    GLuint enable;

    // TODO: properly handle texture_1d, texture_3d, texture_cube
    const GLboolean tex_enabled = tu->flags.texture_1d || tu->flags.texture_2d;
    if (tu->enabled && tex_enabled) {
      enable = NV_TEX_ENABLE;
      if (pbgl.flags.alpha_test)
        enable |= NV_TEX_ALPHAKILL;
    } else {
      enable = NV_TEX_DISABLE;
    }

    const GLuint tidx = i * 64;
    if (tu->tex && enable != NV_TEX_DISABLE) {
      // note: we only operate on swizzled textures
      p = push_command_parameter(p, NV097_SET_TEXTURE_OFFSET + tidx, tu->tex->nv.addr);
      p = push_command_parameter(p, NV097_SET_TEXTURE_FORMAT + tidx, tu->tex->nv.format);
      p = push_command_parameter(p, NV097_SET_TEXTURE_ADDRESS + tidx, tu->tex->nv.wrap);
      p = push_command_parameter(p, NV097_SET_TEXTURE_CONTROL0 + tidx, enable |
        PBGL_MASK(NV097_SET_TEXTURE_CONTROL0_MIN_LOD_CLAMP, 0) |
        PBGL_MASK(NV097_SET_TEXTURE_CONTROL0_MAX_LOD_CLAMP, tu->tex->mipmax - 1));
      p = push_command_parameter(p, NV097_SET_TEXTURE_FILTER + tidx, tu->tex->nv.filter);
      p = push_command_boolean(p, NV097_SET_TEXTURE_MATRIX_ENABLE + i * 4, GL_TRUE);
    } else {
      p = push_command_parameter(p, NV097_SET_TEXTURE_CONTROL0 + tidx, NV_TEX_DISABLE);
      p = push_command_boolean(p, NV097_SET_TEXTURE_MATRIX_ENABLE + i * 4, GL_FALSE);
    }

    tu->dirty = GL_FALSE;
  }

  return p;
}

static inline void mark_active_texunits_dirty(void) {
  for (GLuint i = 0; i < TEXUNIT_COUNT; ++i)
    if (pbgl.tex[i].enabled)
      pbgl.tex[i].dirty = GL_TRUE;
}

GLboolean pbgl_state_flush(void) {
  if (!pbgl.state_dirty) return GL_FALSE;

  GLuint *p = pb_begin();

  if (pbgl.alpha.dirty) {
    // alpha kill might have changed
    for (int i = 0; i < TEXUNIT_COUNT; ++i)
      if (pbgl.tex[i].enabled)
        pbgl.tex[i].dirty = GL_TRUE;
    pbgl.tex_any_dirty = GL_TRUE;
    p = push_command_boolean(p, NV097_SET_ALPHA_TEST_ENABLE, pbgl.flags.alpha_test);
    p = push_command_parameter(p, NV097_SET_ALPHA_FUNC, pbgl.alpha.func);
    p = push_command_parameter(p, NV097_SET_ALPHA_REF, pbgl.alpha.ref);
    pbgl.alpha.dirty = GL_FALSE;
  }

  if (pbgl.blend.dirty) {
    p = push_command_boolean(p, NV097_SET_BLEND_ENABLE, pbgl.flags.blend);
    p = push_command_parameter(p, NV097_SET_BLEND_FUNC_SFACTOR, pbgl.blend.src);
    p = push_command_parameter(p, NV097_SET_BLEND_FUNC_DFACTOR, pbgl.blend.dst);
    p = push_command_parameter(p, NV097_SET_BLEND_EQUATION, pbgl.blend.eqn);
    p = push_command_parameter(p, NV097_SET_BLEND_COLOR, pbgl.blend.color);
    pbgl.blend.dirty = GL_FALSE;
  }

  if (pbgl.depth.dirty) {
    p = push_command_boolean(p, NV097_SET_DEPTH_TEST_ENABLE, pbgl.flags.depth_test);
    p = push_command_boolean(p, NV097_SET_DEPTH_MASK, pbgl.depth.writemask);
    p = push_command_parameter(p, NV097_SET_DEPTH_FUNC, pbgl.depth.func);
    pbgl.depth.dirty = GL_FALSE;
  }

  if (pbgl.stencil.dirty) {
    p = push_command_boolean(p, NV097_SET_STENCIL_TEST_ENABLE, pbgl.flags.stencil_test);
    p = push_command_parameter(p, NV097_SET_STENCIL_OP_FAIL, pbgl.stencil.op_sfail);
    p = push_command_parameter(p, NV097_SET_STENCIL_OP_ZFAIL, pbgl.stencil.op_zfail);
    p = push_command_parameter(p, NV097_SET_STENCIL_OP_ZPASS, pbgl.stencil.op_zpass);
    p = push_command_parameter(p, NV097_SET_STENCIL_FUNC, pbgl.stencil.func);
    p = push_command_parameter(p, NV097_SET_STENCIL_FUNC_MASK, pbgl.stencil.funcmask);
    p = push_command_parameter(p, NV097_SET_STENCIL_MASK, pbgl.stencil.writemask);
    pbgl.stencil.dirty = GL_FALSE;
  }

  if (pbgl.polyofs.dirty) {
    p = push_command_boolean(p, NV097_SET_POLY_OFFSET_FILL_ENABLE, pbgl.flags.poly_offset);
    p = push_command_float(p, NV097_SET_POLYGON_OFFSET_SCALE_FACTOR, pbgl.polyofs.factor);
    p = push_command_float(p, NV097_SET_POLYGON_OFFSET_BIAS, pbgl.polyofs.units);
    pbgl.polyofs.dirty = GL_FALSE;
  }

  if (pbgl.cullface.dirty) {
    p = push_command_parameter(p, NV097_SET_FRONT_FACE, pbgl.cullface.frontface);
    p = push_command_boolean(p, NV097_SET_CULL_FACE_ENABLE, pbgl.flags.cull_face);
    p = push_command_parameter(p, NV097_SET_CULL_FACE, pbgl.cullface.cullface);
    pbgl.cullface.dirty = GL_FALSE;
  }

  if (pbgl.fog.dirty) {
    p = push_command_boolean(p, NV097_SET_FOG_ENABLE, pbgl.flags.fog);
    p = push_command_parameter(p, NV097_SET_FOG_MODE, pbgl.fog.mode);
    p = push_command_parameter(p, NV097_SET_FOG_COLOR, pbgl.fog.color);
    // TODO
    pbgl.fog.dirty = GL_FALSE;
  }

  if (pbgl.colormask.dirty) {
    p = push_command_parameter(p, NV097_SET_COLOR_MASK, pbgl.colormask.value);
    pbgl.colormask.dirty = GL_FALSE;
  }

  if (pbgl.view.dirty) {
    const GLfloat znear = pbgl.view.znear * (GLfloat)PBGL_Z_MAX;
    const GLfloat zfar = pbgl.view.zfar * (GLfloat)PBGL_Z_MAX;
    mat4_viewport(&pbgl.view.mtx, pbgl.view.x, pbgl.view.y, pbgl.view.w, pbgl.view.h, znear, zfar);
    p = push_command_float(p, NV097_SET_CLIP_MIN, znear);
    p = push_command_float(p, NV097_SET_CLIP_MAX, zfar);
    // this goes into the projection matrix, so flag it too
    pbgl.view.dirty = GL_FALSE;
    pbgl.mtx_any_dirty = pbgl.mtx[MTX_PROJECTION].dirty = GL_TRUE;
  }

  if (pbgl.lightmodel.dirty) {
    p = push_command_parameter(p, NV097_SET_LIGHT_CONTROL, 
      PBGL_MASK(NV097_SET_LIGHT_CONTROL_SEPARATE_SPECULAR_EN, GL_FALSE) | // TODO: this is probably a GL setting
      PBGL_MASK(NV097_SET_LIGHT_CONTROL_LOCALEYE, pbgl.lightmodel.local) |
      PBGL_MASK(NV097_SET_LIGHT_CONTROL_SOUT, NV097_SET_LIGHT_CONTROL_SOUT_ZERO_OUT));
    p = push_command_boolean(p, NV097_SET_TWO_SIDE_LIGHT_EN, pbgl.lightmodel.twosided);
    // set both front and back colors just in case
    p = push_command(p, NV097_SET_SCENE_AMBIENT_COLOR, 3);
    p = push_floats(p, pbgl.lightmodel.ambient.v, 3);
    p = push_command(p, NV097_SET_BACK_SCENE_AMBIENT_COLOR, 3);
    p = push_floats(p, pbgl.lightmodel.ambient.v, 3);
    pbgl.lightmodel.dirty = GL_FALSE;
  }

  pb_end(p);

  if (pbgl.light_any_dirty) {
    p = pb_begin();
    p = push_command_boolean(p, NV097_SET_LIGHTING_ENABLE, pbgl.flags.lighting);
    pb_end(p);
    if (pbgl.flags.lighting)
      pbgl_light_flush_all();
    pbgl.light_any_dirty = GL_FALSE;
  }

  if (pbgl.tex_any_dirty) {
    p = pb_begin();
    for (GLuint i = 0; i < TEXUNIT_COUNT; ++i)
      p = flush_texunit(p, i);
    pb_end(p);
    pbgl.tex_any_dirty = GL_FALSE;
  }

  if (pbgl.mtx_any_dirty) {
    p = pb_begin();
    if (pbgl.mtx[MTX_PROJECTION].dirty || pbgl.mtx[MTX_MODELVIEW].dirty) {
      mat4f tmp;
      mat4_mul(&tmp, &pbgl.view.mtx, &pbgl.mtx[MTX_PROJECTION].mtx);
      if (pbgl.mtx[MTX_PROJECTION].dirty)
        p = push_command_matrix4x4_transposed(p, NV097_SET_PROJECTION_MATRIX, tmp.v);
      if (pbgl.mtx[MTX_MODELVIEW].dirty) {
        // inverse modelview matrix only required for lighting, only calculate it when required
        // lighting toggle will set the dirty flag if needed
        if (pbgl.flags.lighting) {
          mat4f invmv;
          mat4_invert(&invmv, &pbgl.mtx[MTX_MODELVIEW].mtx);
          p = push_command_matrix4x4(p, NV097_SET_INVERSE_MODEL_VIEW_MATRIX, invmv.v);
        }
        p = push_command_matrix4x4_transposed(p, NV097_SET_MODEL_VIEW_MATRIX, pbgl.mtx[MTX_MODELVIEW].mtx.v);
      }
      mat4_mul(&tmp, &tmp, &pbgl.mtx[MTX_MODELVIEW].mtx);
      p = push_command_matrix4x4_transposed(p, NV097_SET_COMPOSITE_MATRIX, tmp.v);
      pbgl.mtx[MTX_PROJECTION].dirty = pbgl.mtx[MTX_MODELVIEW].dirty = GL_FALSE;
    }
    pb_end(p);
    if (pbgl.mtx[MTX_TEXTURE].dirty) {
      // push same texture matrix for all textures
      // FIXME: each texunit should have its own
      p = pb_begin();
      for (GLuint i = 0, ofs = 0; i < TEXUNIT_COUNT; ++i, ofs += 4 * 4 * 4)
        if (pbgl.tex[i].enabled)
          p = push_command_matrix4x4(p, NV097_SET_TEXTURE_MATRIX + ofs, pbgl.mtx[MTX_TEXTURE].mtx.v);
      pb_end(p);
      pbgl.mtx[MTX_TEXTURE].dirty = GL_FALSE;
    }
    pbgl.mtx_any_dirty = GL_FALSE;
  }

  if (pbgl.texenv_dirty) {
    p = pb_begin();
    p = pbgl_texenv_push(p);
    pb_end(p);
    pbgl.texenv_dirty = GL_FALSE;
  }

  pbgl.state_dirty = GL_FALSE;

  return GL_TRUE;
}

static inline void set_feature(GLenum feature, GLboolean value) {
  if (pbgl.imm.active) {
    // can't do this during model creation
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  GLuint idx = 0;

  switch (feature) {
    case GL_ALPHA_TEST:
      FLAG_DIRTY_IF_CHANGED(alpha, alpha_test, value);
      pbgl.flags.alpha_test = value;
      break;
    case GL_DEPTH_TEST:
      FLAG_DIRTY_IF_CHANGED(depth, depth_test, value);
      pbgl.flags.depth_test = value;
      break;
    case GL_SCISSOR_TEST:
      FLAG_DIRTY_IF_CHANGED(scissor, scissor_test, value);
      pbgl.flags.scissor_test = value;
      break;
    case GL_STENCIL_TEST:
      FLAG_DIRTY_IF_CHANGED(stencil, stencil_test, value);
      pbgl.flags.stencil_test = value;
      break;
    case GL_BLEND:
      FLAG_DIRTY_IF_CHANGED(blend, blend, value);
      pbgl.flags.blend = value;
      break;
    case GL_CULL_FACE:
      FLAG_DIRTY_IF_CHANGED(cullface, cull_face, value);
      pbgl.flags.cull_face = value;
      break;
    case GL_DITHER:
      // TODO
      pbgl.flags.dither = value;
      break;
    case GL_FOG:
      FLAG_DIRTY_IF_CHANGED(fog, fog, value);
      pbgl.flags.fog = value;
      break;
    case GL_LIGHT0 ... GL_LIGHT7:
      idx = feature - GL_LIGHT0;
      if (pbgl.light[idx].enabled != value)
        pbgl.state_dirty = pbgl.light_any_dirty = pbgl.light[idx].dirty = GL_TRUE;
      pbgl.light[idx].enabled = value;
      break;
    case GL_LIGHTING:
      if (pbgl.flags.lighting != value)
        pbgl.state_dirty = pbgl.light_any_dirty = GL_TRUE;
      pbgl.flags.lighting = value;
      // if we're enabling lighting, we'll need to upload the inverse modelview matrix
      if (value) pbgl.mtx_any_dirty = pbgl.mtx[MTX_MODELVIEW].dirty = GL_TRUE;
      break;
    case GL_POLYGON_OFFSET_FILL:
      FLAG_DIRTY_IF_CHANGED(polyofs, poly_offset, value);
      pbgl.flags.poly_offset = value;
      break;
    case GL_TEXTURE_1D:
      if (pbgl.tex[pbgl.active_tex_sv].flags.texture_1d != value) {
        pbgl.tex_any_dirty = GL_TRUE;
        pbgl.texenv_dirty = GL_TRUE;
        pbgl.state_dirty = GL_TRUE;
        pbgl.tex[pbgl.active_tex_sv].dirty = GL_TRUE;
      }
      pbgl.tex[pbgl.active_tex_sv].flags.texture_1d = value;
      break;
    case GL_TEXTURE_2D:
      if (pbgl.tex[pbgl.active_tex_sv].flags.texture_2d != value) {
        pbgl.tex_any_dirty = GL_TRUE;
        pbgl.texenv_dirty = GL_TRUE;
        pbgl.state_dirty = GL_TRUE;
        pbgl.tex[pbgl.active_tex_sv].dirty = GL_TRUE;
      }
      pbgl.tex[pbgl.active_tex_sv].flags.texture_2d = value;
      break;
    case GL_TEXTURE_GEN_S:
      pbgl.flags.texgen_s = value;
      break;
    case GL_TEXTURE_GEN_T:
      pbgl.flags.texgen_t = value;
      break;
    case GL_TEXTURE_GEN_R:
      pbgl.flags.texgen_r = value;
      break;
    case GL_TEXTURE_GEN_Q:
      pbgl.flags.texgen_q = value;
      break;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      break;
  }
}

static inline void set_client_state(GLenum state, GLboolean value) {
  if (pbgl.imm.active) {
    // can't do this during model creation
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  const GLenum arr = glarr_to_varr(state);
  if (arr == GL_INVALID_ENUM) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  if (arr == VARR_TEXCOORD)
    pbgl.tex[pbgl.active_tex_cl].varray.enabled = value;

  pbgl.varray[arr].enabled = value;
}

/* GL FUNCTIONS BEGIN */

GL_API void glEnable(GLenum feature) {
  set_feature(feature, GL_TRUE);
}

GL_API void glDisable(GLenum feature) {
  set_feature(feature, GL_FALSE);
}

GL_API GLboolean glIsEnabled(GLenum feature) {
  if (pbgl.imm.active) {
    // can't do this during model creation
    pbgl_set_error(GL_INVALID_OPERATION);
    return GL_FALSE;
  }

  switch (feature) {
    case GL_ALPHA_TEST:           return pbgl.flags.alpha_test;
    case GL_DEPTH_TEST:           return pbgl.flags.depth_test;
    case GL_SCISSOR_TEST:         return pbgl.flags.scissor_test;
    case GL_STENCIL_TEST:         return pbgl.flags.stencil_test;
    case GL_BLEND:                return pbgl.flags.blend;
    case GL_CULL_FACE:            return pbgl.flags.cull_face;
    case GL_DITHER:               return pbgl.flags.dither;
    case GL_FOG:                  return pbgl.flags.fog;
    case GL_LIGHT0 ... GL_LIGHT7: return pbgl.light[feature - GL_LIGHT0].enabled;
    case GL_LIGHTING:             return pbgl.flags.lighting;
    case GL_POLYGON_OFFSET_FILL:  return pbgl.flags.poly_offset;
    case GL_TEXTURE_1D:           return pbgl.tex[pbgl.active_tex_sv].flags.texture_1d;
    case GL_TEXTURE_2D:           return pbgl.tex[pbgl.active_tex_sv].flags.texture_2d;
    case GL_TEXTURE_GEN_S:        return pbgl.flags.texgen_s;
    case GL_TEXTURE_GEN_T:        return pbgl.flags.texgen_t;
    case GL_TEXTURE_GEN_R:        return pbgl.flags.texgen_r;
    case GL_TEXTURE_GEN_Q:        return pbgl.flags.texgen_q;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      return GL_FALSE;
  }
}

GL_API void glEnableClientState(GLenum state) {
  set_client_state(state, GL_TRUE);
}

GL_API void glDisableClientState(GLenum state) {
  set_client_state(state, GL_FALSE);
}

// technically all of these should produce various errors, but fuck all those branches

GL_API void glAlphaFunc(GLenum func, GLclampf ref) {
  const GLubyte uref = ref * 255.f;
  if (pbgl.alpha.func != func || pbgl.alpha.ref != uref)
    pbgl.state_dirty = pbgl.alpha.dirty = GL_TRUE;
  pbgl.alpha.func = func & 0xF;
  pbgl.alpha.ref = uref;
}

GL_API void glBlendColor(GLclampf rf, GLclampf gf, GLclampf bf, GLclampf af) {
  const GLubyte r = rf * 255.f;
  const GLubyte g = gf * 255.f;
  const GLubyte b = bf * 255.f;
  const GLubyte a = af * 255.f;
  const GLuint color = (a << 24) | (r << 16) | (g << 8) | b;
  if (pbgl.blend.color != color)
    pbgl.state_dirty = pbgl.blend.dirty = GL_TRUE;
  pbgl.blend.color = color;
}

GL_API void glBlendEquation(GLenum eqn) {
  if (pbgl.blend.eqn != eqn)
      pbgl.state_dirty = pbgl.blend.dirty = GL_TRUE;
  pbgl.blend.eqn = eqn;
}

GL_API void glBlendFunc(GLenum sfactor, GLenum dfactor) {
  if (pbgl.blend.src != sfactor || pbgl.blend.dst != dfactor)
    pbgl.state_dirty = pbgl.blend.dirty = GL_TRUE;
  pbgl.blend.src = sfactor;
  pbgl.blend.dst = dfactor;
}

GL_API void glClearColor(GLclampf rf, GLclampf gf, GLclampf bf, GLclampf af) {
  const GLubyte r = rf * 255.f;
  const GLubyte g = gf * 255.f;
  const GLubyte b = bf * 255.f;
  const GLubyte a = af * 255.f;
  pbgl.clear_color = (a << 24) | (r << 16) | (g << 8) | b;
}

GL_API void glColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a) {
  const GLuint oldmask = pbgl.colormask.value;
  pbgl.colormask.value = 0;
  if (r) pbgl.colormask.value |= NV097_SET_COLOR_MASK_RED_WRITE_ENABLE;
  if (g) pbgl.colormask.value |= NV097_SET_COLOR_MASK_GREEN_WRITE_ENABLE;
  if (b) pbgl.colormask.value |= NV097_SET_COLOR_MASK_BLUE_WRITE_ENABLE;
  if (a) pbgl.colormask.value |= NV097_SET_COLOR_MASK_ALPHA_WRITE_ENABLE;
  if (oldmask != pbgl.colormask.value)
    pbgl.state_dirty = pbgl.colormask.dirty = GL_TRUE;
}

GL_API void glCullFace(GLenum face) {
  if (pbgl.cullface.cullface != face)
    pbgl.state_dirty = pbgl.cullface.dirty = GL_TRUE;
  pbgl.cullface.cullface = face;
}

GL_API void glDepthFunc(GLenum func) {
  if (pbgl.depth.func != func)
    pbgl.state_dirty = pbgl.depth.dirty = GL_TRUE;
  pbgl.depth.func = func & 0xF;
}

GL_API void glDepthMask(GLboolean write) {
  if (pbgl.depth.writemask != write)
    pbgl.state_dirty = pbgl.depth.dirty = GL_TRUE;
  pbgl.depth.writemask = write;
}

GL_API void glDepthRange(GLclampd znear, GLclampd zfar) {
  if (pbgl.view.znear != znear || pbgl.view.zfar != zfar)
    pbgl.state_dirty = pbgl.view.dirty = GL_TRUE;
  pbgl.view.znear = znear;
  pbgl.view.zfar = zfar;
}

GL_API void glFrontFace(GLenum face) {
  if (pbgl.cullface.frontface != face)
    pbgl.state_dirty = pbgl.cullface.dirty = GL_TRUE;
  pbgl.cullface.frontface = face;
}

GL_API void glLineWidth(GLfloat width) {
  // TODO:
}

GL_API void glPixelStorei(GLenum pname, GLint param) {
  // TODO:
}

GL_API void glPolygonMode(GLenum face, GLenum mode) {
  // TODO:
}

GL_API void glPointSize(GLfloat size) {
  // TODO:
}

GL_API void glPolygonOffset(GLfloat factor, GLfloat units) {
  if (pbgl.polyofs.factor != factor || pbgl.polyofs.units != units)
    pbgl.state_dirty = pbgl.polyofs.dirty = GL_TRUE;
  pbgl.polyofs.factor = factor;
  pbgl.polyofs.units = units;
}

GL_API void glScissor(GLint x, GLint y, GLint width, GLint height) {
  // TODO:
}

GL_API void glShadeModel(GLenum model) {
  // TODO
}

GL_API void glStencilFunc(GLenum func, GLint ref, GLuint mask) {
  if (pbgl.stencil.func != func || pbgl.stencil.ref != ref || pbgl.stencil.funcmask != mask)
    pbgl.state_dirty = pbgl.stencil.dirty = GL_TRUE;
  pbgl.stencil.func = func & 0xF;
  pbgl.stencil.ref = ref;
  pbgl.stencil.funcmask = mask;
}

GL_API void glStencilMask(GLuint mask) {
  if (pbgl.stencil.writemask != mask)
    pbgl.state_dirty = pbgl.stencil.dirty = GL_TRUE;
  pbgl.stencil.writemask = mask;
}

GL_API void glStencilOp(GLenum sfail, GLenum zfail, GLenum zpass) {
  if (pbgl.stencil.op_sfail != sfail || pbgl.stencil.op_zfail != zfail || pbgl.stencil.op_zpass != zpass)
    pbgl.state_dirty = pbgl.stencil.dirty = GL_TRUE;
  pbgl.stencil.op_sfail = sfail;
  pbgl.stencil.op_zfail = zfail;
  pbgl.stencil.op_zpass = zpass;
}

GL_API void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
  if (pbgl.view.x != x || pbgl.view.y != y || pbgl.view.w != width || pbgl.view.h != height)
    pbgl.state_dirty = pbgl.view.dirty = GL_TRUE;
  pbgl.view.x = x;
  pbgl.view.y = y;
  pbgl.view.w = width;
  pbgl.view.h = height;
}
