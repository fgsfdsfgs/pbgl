#ifndef _PBGL_TYPES_H
#define _PBGL_TYPES_H

#define PBGL_Z_MAX 16777215.0

typedef union {
  float v[2];
  struct { float x, y; };
} vec2f;

typedef union {
  float v[3];
  struct { float x, y, z; };
  struct { float r, g, b; };
  vec2f vec2;
} vec3f;

typedef union {
  float v[4];
  struct { float x, y, z, w; };
  struct { float r, g, b, a; };
  vec2f vec2;
  vec3f vec3;
} vec4f;

typedef union {
  float v[16];
  float m[4][4];
  vec4f rows[4];
} mat4f;

extern const vec4f vec4_one;
extern const vec4f vec4_zero;

extern const mat4f mat4_identity;
extern const mat4f mat4_zero;

void mat4_viewport(mat4f *m, float x, float y, float w, float h, float znear, float zfar);
mat4f *mat4_mul(mat4f *c, const mat4f *a, const mat4f *b);
mat4f *mat4_invert(mat4f *out, const mat4f *m);

#endif // _PBGL_TYPES_H
