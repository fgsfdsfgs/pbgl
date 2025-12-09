#ifndef _PBGL_UTILS_H
#define _PBGL_UTILS_H

extern const unsigned int ulog2_tab[32];

static inline unsigned int ulog2(unsigned int value) {
  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  value |= value >> 8;
  value |= value >> 16;
  return ulog2_tab[(unsigned int)(value * 0x07C4ACDD) >> 27];
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
