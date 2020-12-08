#ifndef _PBGL_LIGHT_H
#define _PBGL_LIGHT_H

#define LIGHT_COUNT 8

enum lightfactor_e {
  LFACTOR_CONSTANT,
  LFACTOR_LINEAR,
  LFACTOR_QUADRATIC,

  LFACTOR_COUNT
};

void pbgl_light_flush_all(void);

#endif // _PBGL_LIGHT_H
