#ifndef _PBGL_ERROR_H
#define _PBGL_ERROR_H

#include "GL/gl.h"
#include "GL/glext.h"

#ifdef PBGL_DEBUG
# define pbgl_set_error(x) pbgl_set_gl_error(x, 0, __func__, __LINE__)
# define pbgl_set_error_arg(x, y) pbgl_set_gl_error(x, y, __func__, __LINE__)
#else
# define pbgl_set_error(x) pbgl_set_gl_error(x, 0, NULL, 0)
# define pbgl_set_error_arg(x, y) pbgl_set_gl_error(x, y, NULL, 0)
#endif

void pbgl_set_gl_error(GLenum error, GLenum arg, const char *func, const int line);

#ifdef PBGL_DEBUG
void pbgl_debug_log(const char *fmt, ...);
const char *pbgl_get_error_str(GLenum error);
#else
# define pbgl_debug_log(...)
#endif

#endif // _PBGL_ERROR_H
