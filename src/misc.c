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
  if ((format != GL_RGB && format != GL_RGBA) || type != GL_UNSIGNED_BYTE) {
    // TODO: support other formats
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  if (!pixels || x < 0 || y < 0 || x + width > pbgl.fb_width || y + height > pbgl.fb_height) {
    pbgl_set_error(GL_INVALID_VALUE);
    return;
  }

  if (!width || !height)
    return; // no-op

  glFlush();

  const GLuint src_bytespp = 4; // TODO: support other formats
  const GLuint src_pitch = pb_back_buffer_pitch();

  const GLuint dst_bytespp = (format == GL_RGB) ? 3 : 4;
  GLuint dst_pitch = width * dst_bytespp;
  if (pbgl.pack_align > 1)
    dst_pitch = (dst_pitch + pbgl.pack_align - 1) & ~(pbgl.pack_align - 1);

  const GLubyte *src_line = (GLubyte*)pb_back_buffer() + y * src_pitch + x * src_bytespp;
  GLubyte *dst_line = (GLubyte*)pixels + (height - 1) * dst_pitch; // we have to flip the image, so output it upside down

  const GLboolean dst_alpha = (format == GL_RGBA);
  for (GLint i = 0; i < height; ++i, src_line += src_pitch, dst_line -= dst_pitch) {
    const GLubyte *src = src_line;
    GLubyte *dst = dst_line;
    for (GLint j = 0; j < width; ++j, dst += dst_bytespp, src += src_bytespp) {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      if (dst_alpha)
        dst[3] = src[3];
    }
  }
}

// TODO: figure out how the fuck to do this properly

GL_API void glFinish(void) {
  glFlush();
  while (pb_busy());
}

GL_API void glFlush(void) {
  pbgl_state_flush();
}
