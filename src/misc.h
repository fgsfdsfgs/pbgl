#ifndef _PBGL_MISC_H
#define _PBGL_MISC_H

#include "GL/gl.h"
#include "GL/glext.h"

#define PBGL_MASK(mask, val) (((val) << (__builtin_ffs(mask)-1)) & (mask))

static inline GLuint umax(const GLuint a, const GLuint b) {
  return (a > b) ? a : b;
}

static inline GLuint umin(const GLuint a, const GLuint b) {
  return (a < b) ? a : b;
}

static inline GLint imax(const GLint a, const GLint b) {
  return (a > b) ? a : b;
}

static inline GLint imin(const GLint a, const GLint b) {
  return (a < b) ? a : b;
}


#endif // _PBGL_MISC_H
