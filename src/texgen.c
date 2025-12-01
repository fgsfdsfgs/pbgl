#include <pbkit/pbkit.h>

#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "error.h"
#include "texture.h"
#include "state.h"
#include "misc.h"

// NOTE: all the GL texgen mode values are the same as the nv2a register values for them

/* pbgl internals */

GLuint *pbgl_texgen_push(GLuint *p) {
  for (GLuint i = 0; i < TEXUNIT_COUNT; ++i) {
    texgen_state_t *texgen = pbgl.texgen + i;

    // TODO: properly handle texture_1d, texture_3d, texture_cube
    const GLboolean tex_enabled =
      pbgl.tex[i].flags.texture_1d ||
      pbgl.tex[i].flags.texture_2d;

    if (texgen->dirty) {
      texgen->dirty = GL_FALSE;
      for (GLuint j = 0; j < 16; j += 4) {
        if (tex_enabled && texgen->coord[j].enabled) {
          p = pb_push1(p, NV097_SET_TEXGEN_S + j, texgen->coord[j].mode);
          if (texgen->coord[j].mode == GL_EYE_LINEAR)
            p = pb_push4fv(p, NV097_SET_TEXGEN_PLANE_S + j * 4, texgen->coord[j].eye_plane.v);
          else if (texgen->coord[j].mode == GL_OBJECT_LINEAR)
            p = pb_push4fv(p, NV097_SET_TEXGEN_PLANE_S + j * 4, texgen->coord[j].obj_plane.v);
        } else {
          p = pb_push1(p, NV097_SET_TEXGEN_S + j, NV097_SET_TEXGEN_S_DISABLE);
        }
      }
    }
  }

  return p;
}

/* GL FUNCTIONS BEGIN */

GL_API void glTexGeni(GLenum coord, GLenum pname, GLint param) {
  if (pname != GL_TEXTURE_GEN_MODE) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  if (coord < GL_S || coord > GL_Q) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  if (param != GL_OBJECT_LINEAR && param != GL_EYE_LINEAR && param != GL_SPHERE_MAP &&
      param != GL_NORMAL_MAP && param != GL_REFLECTION_MAP) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  coord -= GL_S;
  if (pbgl.texgen[pbgl.active_tex_sv].coord[coord].mode != param) {
    pbgl.texgen[pbgl.active_tex_sv].coord[coord].mode = param;
    pbgl.state_dirty = pbgl.texgen[pbgl.active_tex_sv].dirty = pbgl.texgen_dirty = GL_TRUE;
  }
}

GL_API void glTexGenf(GLenum coord, GLenum pname, GLfloat param) {
  glTexGeni(coord, pname, (GLint)param);
}

GL_API void glTexGend(GLenum coord, GLenum pname, GLdouble param) {
  glTexGeni(coord, pname, (GLint)param);
}

GL_API void glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params) {
  if (!params) {
    pbgl_set_error(GL_INVALID_VALUE);
    return;
  }

  if (pname == GL_TEXTURE_GEN_MODE) {
    glTexGenf(coord, pname, params[0]);
    return;
  }

  if (coord < GL_S || coord > GL_Q) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  coord -= GL_S;

  switch (pname) {
    case GL_EYE_PLANE:
      memcpy(pbgl.texgen[pbgl.active_tex_sv].coord[coord].eye_plane.v, params, sizeof(vec4f));
      pbgl.state_dirty = pbgl.texgen_dirty = pbgl.texgen[pbgl.active_tex_sv].dirty = GL_TRUE;
      break;
    case GL_OBJECT_PLANE:
      memcpy(pbgl.texgen[pbgl.active_tex_sv].coord[coord].obj_plane.v, params, sizeof(vec4f));
      pbgl.state_dirty = pbgl.texgen_dirty = pbgl.texgen[pbgl.active_tex_sv].dirty = GL_TRUE;
      break;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      break;
  }
}

GL_API void glTexGeniv(GLenum coord, GLenum pname, const GLint *params) {
  if (!params) {
    pbgl_set_error(GL_INVALID_VALUE);
    return;
  }

  if (pname == GL_TEXTURE_GEN_MODE) {
    glTexGeni(coord, pname, params[0]);
    return;
  }

  const GLfloat fparams[4] = {
    (GLfloat)params[0],
    (GLfloat)params[1],
    (GLfloat)params[2],
    (GLfloat)params[3]
  };

  glTexGenfv(coord, pname, fparams);
}
