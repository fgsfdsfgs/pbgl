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

static inline GLuint ulog2(const GLuint value) {
  return (31 - __builtin_clz(value | 1));
}

static inline GLuint uflp2(GLuint x) {
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x - (x >> 1);
}

#endif // _PBGL_MISC_H
