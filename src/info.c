#include <pbkit/pbkit.h>
#include <string.h>

#include "GL/gl.h"
#include "GL/glext.h"
#include "error.h"
#include "array.h"
#include "matrix.h"
#include "texture.h"
#include "state.h"
#include "info.h"

#define xstr(a) str(a)
#define str(a) #a

static const char *gl_version = xstr(PBGL_GL_VERSION) " " xstr(PBGL_VERSION);
static const char *gl_renderer = "pbGL";
static const char *gl_vendor = "not NVIDIA";
static const char *gl_ext_string = \
  "GL_ARB_multitexture " \
  "GL_ARB_texture_env_add " \
  "GL_ARB_texture_env_combine " \
  "GL_PBGL_texture_generate_mipmap";

/* GL FUNCTIONS BEGIN */

GL_API const GLubyte *glGetString(GLenum pname) {
  switch (pname) {
    case GL_VENDOR:     return (const GLubyte *)gl_vendor;
    case GL_RENDERER:   return (const GLubyte *)gl_renderer;
    case GL_VERSION:    return (const GLubyte *)gl_version;
    case GL_EXTENSIONS: return (const GLubyte *)gl_ext_string;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      return NULL;
  }
}

GL_API void glGetBooleanv(GLenum pname, GLboolean *params) {
  switch (pname) {
    case GL_ALPHA_TEST:            *params = pbgl.flags.alpha_test; break;
    case GL_BLEND:                 *params = pbgl.flags.blend; break;
    case GL_COLOR_ARRAY:           *params = pbgl.varray[VARR_COLOR1].enabled; break;
    case GL_CULL_FACE:             *params = pbgl.flags.cull_face; break;
    case GL_DEPTH_TEST:            *params = pbgl.flags.depth_test; break;
    case GL_DEPTH_WRITEMASK:       *params = pbgl.depth.writemask; break;
    case GL_DITHER:                *params = pbgl.flags.dither; break;
    case GL_FOG:                   *params = pbgl.flags.fog; break;
    case GL_FOG_COORD_ARRAY:       *params = pbgl.varray[VARR_FOG].enabled; break;
    // case GL_INDEX_ARRAY:           *params = pbgl.varray[VARR_INDEX].enabled; break;
    case GL_LIGHTING:              *params = pbgl.flags.lighting; break;
    case GL_NORMAL_ARRAY:          *params = pbgl.varray[VARR_NORMAL].enabled; break;
    case GL_POLYGON_OFFSET_FILL:   *params = pbgl.flags.poly_offset; break;
    case GL_SCISSOR_TEST:          *params = pbgl.flags.scissor_test; break;
    case GL_SECONDARY_COLOR_ARRAY: *params = pbgl.varray[VARR_COLOR2].enabled; break;
    case GL_STENCIL_TEST:          *params = pbgl.flags.stencil_test; break;
    case GL_TEXTURE_1D:            *params = pbgl.tex[pbgl.active_tex_sv].flags.texture_1d; break;
    case GL_TEXTURE_2D:            *params = pbgl.tex[pbgl.active_tex_sv].flags.texture_2d; break;
    case GL_TEXTURE_3D:            *params = pbgl.tex[pbgl.active_tex_sv].flags.texture_3d; break;
    case GL_TEXTURE_COORD_ARRAY:   *params = pbgl.varray[VARR_TEXCOORD].enabled; break;
    case GL_TEXTURE_GEN_Q:         *params = pbgl.flags.texgen_q; break;
    case GL_TEXTURE_GEN_R:         *params = pbgl.flags.texgen_r; break;
    case GL_TEXTURE_GEN_S:         *params = pbgl.flags.texgen_s; break;
    case GL_TEXTURE_GEN_T:         *params = pbgl.flags.texgen_t; break;
    case GL_VERTEX_ARRAY:          *params = pbgl.varray[VARR_POSITION].enabled; break;
    case GL_COLOR_WRITEMASK:
      params[0] = (pbgl.colormask.value & NV097_SET_COLOR_MASK_RED_WRITE_ENABLE) != 0;
      params[1] = (pbgl.colormask.value & NV097_SET_COLOR_MASK_GREEN_WRITE_ENABLE) != 0;
      params[2] = (pbgl.colormask.value & NV097_SET_COLOR_MASK_BLUE_WRITE_ENABLE) != 0;
      params[3] = (pbgl.colormask.value & NV097_SET_COLOR_MASK_ALPHA_WRITE_ENABLE) != 0;
      break;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      break;
  }
}

GL_API void glGetIntegerv(GLenum pname, GLint *params) {
  switch (pname) {
    case GL_ACTIVE_TEXTURE:             *params = pbgl.active_tex_sv + GL_TEXTURE0; break;
    case GL_ALPHA_TEST_FUNC:            *params = pbgl.alpha.func + 0x200; break;
    case GL_ALPHA_BITS:                 *params = 8; break; // TODO: read from current video mode?
    case GL_BLUE_BITS:                  *params = 8; break; // TODO: read from current video mode?
    case GL_CLIENT_ACTIVE_TEXTURE:      *params = pbgl.active_tex_cl + GL_TEXTURE0; break;
    case GL_CULL_FACE_MODE:             *params = pbgl.cullface.cullface; break;
    case GL_DEPTH_BITS:                 *params = 24; break; // TODO: read from current video mode?
    case GL_DEPTH_FUNC:                 *params = pbgl.depth.func + 0x200; break;
    case GL_FRONT_FACE:                 *params = pbgl.cullface.frontface; break;
    case GL_GREEN_BITS:                 *params = 8; break; // TODO: read from current video mode?
    case GL_MATRIX_MODE:                *params = pbgl.mtx_current; break;
    case GL_MAX_MODELVIEW_STACK_DEPTH:  *params = MTX_STACK_SIZE_MV; break;
    case GL_MAX_PROJECTION_STACK_DEPTH: *params = MTX_STACK_SIZE_P; break;
    case GL_MAX_TEXTURE_UNITS:          *params = TEXUNIT_COUNT; break;
    case GL_MAX_TEXTURE_SIZE:           *params = TEX_MAX_SIZE; break;
    case GL_MAX_TEXTURE_STACK_DEPTH:    *params = MTX_STACK_SIZE_T; break;
    case GL_MAX_TEXTURE_LOD_BIAS:       *params = TEX_MAX_MIPS - 1; break;
    case GL_MODELVIEW_STACK_DEPTH:      *params = pbgl_mtx_stack_depth(MTX_MODELVIEW); break;
    case GL_PROJECTION_STACK_DEPTH:     *params = pbgl_mtx_stack_depth(MTX_PROJECTION); break;
    case GL_RED_BITS:                   *params = 8; break; // TODO: read from current video mode?
    case GL_SHADE_MODEL:                *params = GL_SMOOTH; break; // TODO
    case GL_STENCIL_BITS:               *params = 8; break; // TODO: read from current video mode?
    case GL_STENCIL_FUNC:               *params = pbgl.stencil.func + 0x200; break;
    case GL_TEXTURE_STACK_DEPTH:        *params = pbgl_mtx_stack_depth(MTX_TEXTURE); break;
    case GL_CURRENT_COLOR:
      params[0] = pbgl.varray[VARR_COLOR1].value.r * 255.f;
      params[1] = pbgl.varray[VARR_COLOR1].value.g * 255.f;
      params[2] = pbgl.varray[VARR_COLOR1].value.b * 255.f;
      params[3] = pbgl.varray[VARR_COLOR1].value.a * 255.f;
      break;
    default:
      // HACK: this should work cause it will set the first byte
      glGetBooleanv(pname, (GLboolean *)params);
      break;
  }
}

GL_API void glGetFloatv(GLenum pname, GLfloat *params) {
  switch (pname) {
    case GL_CURRENT_COLOR:
      params[0] = pbgl.varray[VARR_COLOR1].value.r;
      params[1] = pbgl.varray[VARR_COLOR1].value.g;
      params[2] = pbgl.varray[VARR_COLOR1].value.b;
      params[3] = pbgl.varray[VARR_COLOR1].value.a;
      break;
    case GL_CURRENT_FOG_COORD:
      params[0] = pbgl.varray[VARR_FOG].value.x;
      break;
    case GL_CURRENT_NORMAL:
      params[0] = pbgl.varray[VARR_NORMAL].value.x;
      params[1] = pbgl.varray[VARR_NORMAL].value.y;
      params[2] = pbgl.varray[VARR_NORMAL].value.z;
      break;
    case GL_CURRENT_TEXTURE_COORDS:
      params[0] = pbgl.varray[VARR_TEXCOORD].value.x;
      params[1] = pbgl.varray[VARR_TEXCOORD].value.y;
      params[2] = pbgl.varray[VARR_TEXCOORD].value.z;
      params[3] = pbgl.varray[VARR_TEXCOORD].value.w;
      break;
    case GL_MODELVIEW_MATRIX:
      memcpy(params, pbgl.mtx[MTX_MODELVIEW].mtx.v, sizeof(GLfloat) * 16);
      break;
    case GL_PROJECTION_MATRIX:
      memcpy(params, pbgl.mtx[MTX_PROJECTION].mtx.v, sizeof(GLfloat) * 16);
      break;
    case GL_TEXTURE_MATRIX:
      memcpy(params, pbgl.mtx[MTX_TEXTURE].mtx.v, sizeof(GLfloat) * 16);
      break;
    default:
      // by spec we should bother to convert other params to floats, but who fucking cares?
      pbgl_set_error(GL_INVALID_ENUM);
      break;
  }
}

GL_API void glHint(GLenum target, GLenum mode) {
  // TODO:
}
