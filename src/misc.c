#include <pbkit/pbkit.h>

#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "error.h"
#include "push.h"
#include "state.h"

/* GL FUNCTIONS BEGIN */

GL_API void glClear(GLbitfield mask) {
  GLuint nvmask = 0;
  if (mask & GL_COLOR_BUFFER_BIT)   nvmask |= NV097_CLEAR_SURFACE_COLOR;
  if (mask & GL_DEPTH_BUFFER_BIT)   nvmask |= NV097_CLEAR_SURFACE_Z;
  if (mask & GL_STENCIL_BUFFER_BIT) nvmask |= NV097_CLEAR_SURFACE_STENCIL;
  GLuint *p = pb_begin();
  p = push_command_parameter(p, NV097_SET_COLOR_CLEAR_VALUE, pbgl.clear_color);
  // for some reason it wants me to set the clear value every time immediately before the clear
  p = push_command_parameter(p, NV097_SET_ZSTENCIL_CLEAR_VALUE, pbgl.clear_zstencil);
  p = push_command_parameter(p, NV097_CLEAR_SURFACE, nvmask);
  pb_end(p);
}

GL_API void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels) {
  // TODO:
}

// TODO: figure out how the fuck to do this properly

GL_API void glFinish(void) {
  while (pb_busy());
}

GL_API void glFlush(void) {
  // ?
}
