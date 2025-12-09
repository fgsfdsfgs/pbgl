#ifndef _PBGL_UTILS_H
#define _PBGL_UTILS_H

static inline unsigned int ulog2(unsigned int value) {
  return (31 - __builtin_clz(value | 1));
}

static inline unsigned int uflp2(unsigned int x) {
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x - (x >> 1);
}

#endif
