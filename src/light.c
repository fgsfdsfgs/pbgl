#include <stdlib.h>
#include <pbkit/pbkit.h>

#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "error.h"
#include "state.h"
#include "misc.h"
#include "push.h"
#include "light.h"

/* pbgl internals */

void pbgl_light_flush_all(void) {
  // TODO: spot lights
  GLuint lightmask = 0;
  GLuint *p = NULL;

  for (GLuint i = 0, lofs = 0, blofs = 0; i < LIGHT_COUNT; ++i, lofs += 128, blofs += 64) {
    if (pbgl.light[i].enabled) {
      // by GL spec if position.w == 0, then the light is directional and position is direction
      if (pbgl.light[i].pos.w == 0.f)
        lightmask |= NV097_SET_LIGHT_ENABLE_MASK_LIGHT0_INFINITE << (i * 2);
      else
        lightmask |= NV097_SET_LIGHT_ENABLE_MASK_LIGHT0_LOCAL << (i * 2);
      // lights are too thicc, can't push them all in one begin-end pair
      // FIXME: we set both back and front light colors; can GL do them separately?
      if (pbgl.light[i].dirty) {
        p = pb_begin();
        p = push_command(p, NV097_SET_LIGHT_AMBIENT_COLOR + lofs, 3);
        p = push_floats(p, pbgl.light[i].ambient.v, 3);
        p = push_command(p, NV097_SET_BACK_LIGHT_AMBIENT_COLOR + blofs, 3);
        p = push_floats(p, pbgl.light[i].ambient.v, 3);
        p = push_command(p, NV097_SET_LIGHT_DIFFUSE_COLOR + lofs, 3);
        p = push_floats(p, pbgl.light[i].diffuse.v, 3);
        p = push_command(p, NV097_SET_BACK_LIGHT_DIFFUSE_COLOR + blofs, 3);
        p = push_floats(p, pbgl.light[i].diffuse.v, 3);
        p = push_command(p, NV097_SET_LIGHT_SPECULAR_COLOR + lofs, 3);
        p = push_floats(p, pbgl.light[i].specular.v, 3);
        p = push_command(p, NV097_SET_BACK_LIGHT_SPECULAR_COLOR + blofs, 3);
        p = push_floats(p, pbgl.light[i].specular.v, 3);
        p = push_command(p, NV097_SET_LIGHT_LOCAL_ATTENUATION + lofs, 3);
        p = push_floats(p, pbgl.light[i].factor, 3);
        // push direction if directional, position otherwise
        if (pbgl.light[i].pos.w == 0.f) {
          p = push_command(p, NV097_SET_LIGHT_INFINITE_DIRECTION + lofs, 3);
          p = push_floats(p, pbgl.light[i].pos.v, 3);
        } else {
          p = push_command(p, NV097_SET_LIGHT_LOCAL_POSITION + lofs, 3);
          p = push_floats(p, pbgl.light[i].pos.v, 3);
        }
        pb_end(p);
        pbgl.light[i].dirty = GL_FALSE;
      }
    }
  }

  p = pb_begin();
  p = push_command_parameter(p, NV097_SET_LIGHT_ENABLE_MASK, lightmask);
  pb_end(p);
}

/* GL FUNCTIONS BEGIN */

GL_API void glLighti(GLenum light, GLenum pname, GLint param) {
  if (light >= GL_LIGHT0 + LIGHT_COUNT) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  // TODO
  pbgl_set_error(GL_INVALID_ENUM);
}

GL_API void glLightiv(GLenum light, GLenum pname, const GLint *params) {
  // TODO
  glLighti(light, pname, *params);
}

GL_API void glLightf(GLenum light, GLenum pname, GLfloat param) {
  if (light >= GL_LIGHT0 + LIGHT_COUNT) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  const GLuint lidx = light - GL_LIGHT0;

  switch (pname) {
    case GL_CONSTANT_ATTENUATION:
    case GL_LINEAR_ATTENUATION:
    case GL_QUADRATIC_ATTENUATION:
      pbgl.light[lidx].factor[pname - GL_CONSTANT_ATTENUATION] = param;
      break;
    default:
      glLighti(light, pname, (GLint)param);
      return;
  }

  pbgl.state_dirty = pbgl.light_any_dirty = pbgl.light[lidx].dirty = GL_TRUE;
}

GL_API void glLightfv(GLenum light, GLenum pname, const GLfloat *params) {
  if (light >= GL_LIGHT0 + LIGHT_COUNT) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  const GLuint lidx = light - GL_LIGHT0;

  switch (pname) {
    case GL_AMBIENT:
      pbgl.light[lidx].ambient = *(const vec4f *)params;
      break;
    case GL_DIFFUSE:
      pbgl.light[lidx].diffuse = *(const vec4f *)params;
      break;
    case GL_SPECULAR:
      pbgl.light[lidx].specular = *(const vec4f *)params;
      break;
    case GL_POSITION:
      pbgl.light[lidx].pos = *(const vec4f *)params;
      break;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      return;
  }

  pbgl.state_dirty = pbgl.light_any_dirty = pbgl.light[lidx].dirty = GL_TRUE;
}

GL_API void glLightModeli(GLenum pname, GLint param) {
  switch(pname) {
    case GL_LIGHT_MODEL_TWO_SIDE:
      pbgl.lightmodel.twosided = !!param;
      break;
    case GL_LIGHT_MODEL_LOCAL_VIEWER:
      pbgl.lightmodel.local = !!param;
      break;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      return;
  }

  pbgl.state_dirty = pbgl.lightmodel.dirty = GL_TRUE;
}

GL_API void glLightModeliv(GLenum pname, const GLint *params) {
  // TODO
  glLightModeli(pname, *params);
}

GL_API void glLightModelf(GLenum pname, GLfloat param) {
  // TODO
  glLightModeli(pname, (GLint)param);
}

GL_API void glLightModelfv(GLenum pname, const GLfloat *params) {
  switch(pname) {
  case GL_LIGHT_MODEL_AMBIENT:
    pbgl.lightmodel.ambient.r = params[0];
    pbgl.lightmodel.ambient.g = params[1];
    pbgl.lightmodel.ambient.b = params[2];
    pbgl.lightmodel.ambient.a = params[3];
    break;
  default:
    // TODO
    glLightModelf(pname, *params);
    return;
  }

  pbgl.state_dirty = pbgl.lightmodel.dirty = GL_TRUE;
}
