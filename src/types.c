#include <x86intrin.h>
#include "types.h"

const vec4f vec4_zero = {{ 0.f }};
const vec4f vec4_one = {{ 1.f, 1.f, 1.f, 1.f }};

const mat4f mat4_zero = {{ 0.f }};
const mat4f mat4_identity = {
  .m = {
    { 1.f, 0.f, 0.f, 0.f },
    { 0.f, 1.f, 0.f, 0.f },
    { 0.f, 0.f, 1.f, 0.f },
    { 0.f, 0.f, 0.f, 1.f },
  }
};

mat4f *mat4_mul(mat4f *c, const mat4f *a, const mat4f *b) {
  /* FIXME: alignment required
  __m128 row[4], sum[4];
  row[0] = _mm_load_ps(a->m[0]);
  row[1] = _mm_load_ps(a->m[1]);
  row[2] = _mm_load_ps(a->m[2]);
  row[3] = _mm_load_ps(a->m[3]);
  for(int i = 0; i < 4; i++) {
    sum[i] = _mm_setzero_ps();
    sum[i] = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(b->m[i][0]), row[0]), sum[i]);
    sum[i] = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(b->m[i][1]), row[1]), sum[i]);
    sum[i] = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(b->m[i][2]), row[2]), sum[i]);
    sum[i] = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(b->m[i][3]), row[3]), sum[i]);
  }
  _mm_store_ps(c->m[0], sum[0]);
  _mm_store_ps(c->m[1], sum[1]);
  _mm_store_ps(c->m[2], sum[2]);
  _mm_store_ps(c->m[3], sum[3]);
  */

  const float a00 = a->v[ 0], a01 = a->v[ 1], a02 = a->v[ 2], a03 = a->v[ 3],
              a10 = a->v[ 4], a11 = a->v[ 5], a12 = a->v[ 6], a13 = a->v[ 7],
              a20 = a->v[ 8], a21 = a->v[ 9], a22 = a->v[10], a23 = a->v[11],
              a30 = a->v[12], a31 = a->v[13], a32 = a->v[14], a33 = a->v[15];

  float b0 = b->v[0], b1 = b->v[1], b2 = b->v[2], b3 = b->v[3];
  c->v[ 0] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
  c->v[ 1] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
  c->v[ 2] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
  c->v[ 3] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

  b0 = b->v[4]; b1 = b->v[5]; b2 = b->v[6]; b3 = b->v[7];
  c->v[ 4] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
  c->v[ 5] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
  c->v[ 6] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
  c->v[ 7] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

  b0 = b->v[8]; b1 = b->v[9]; b2 = b->v[10]; b3 = b->v[11];
  c->v[ 8] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
  c->v[ 9] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
  c->v[10] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
  c->v[11] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

  b0 = b->v[12]; b1 = b->v[13]; b2 = b->v[14]; b3 = b->v[15];
  c->v[12] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
  c->v[13] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
  c->v[14] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
  c->v[15] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

  return c;
}

mat4f *mat4_invert(mat4f *out, const mat4f *m) {
  out->v[ 0] =  m->v[ 5]*m->v[10]*m->v[15] - m->v[ 5]*m->v[14]*m->v[11] - m->v[ 6]*m->v[ 9]*m->v[15] + m->v[ 6]*m->v[13]*m->v[11] + m->v[ 7]*m->v[ 9]*m->v[14] - m->v[ 7]*m->v[13]*m->v[10];
  out->v[ 1] = -m->v[ 1]*m->v[10]*m->v[15] + m->v[ 1]*m->v[14]*m->v[11] + m->v[ 2]*m->v[ 9]*m->v[15] - m->v[ 2]*m->v[13]*m->v[11] - m->v[ 3]*m->v[ 9]*m->v[14] + m->v[ 3]*m->v[13]*m->v[10];
  out->v[ 2] =  m->v[ 1]*m->v[ 6]*m->v[15] - m->v[ 1]*m->v[14]*m->v[ 7] - m->v[ 2]*m->v[ 5]*m->v[15] + m->v[ 2]*m->v[13]*m->v[ 7] + m->v[ 3]*m->v[ 5]*m->v[14] - m->v[ 3]*m->v[13]*m->v[ 6];
  out->v[ 3] = -m->v[ 1]*m->v[ 6]*m->v[11] + m->v[ 1]*m->v[10]*m->v[ 7] + m->v[ 2]*m->v[ 5]*m->v[11] - m->v[ 2]*m->v[ 9]*m->v[ 7] - m->v[ 3]*m->v[ 5]*m->v[10] + m->v[ 3]*m->v[ 9]*m->v[ 6];

  out->v[ 4] = -m->v[ 4]*m->v[10]*m->v[15] + m->v[ 4]*m->v[14]*m->v[11] + m->v[ 6]*m->v[ 8]*m->v[15] - m->v[ 6]*m->v[12]*m->v[11] - m->v[ 7]*m->v[ 8]*m->v[14] + m->v[ 7]*m->v[12]*m->v[10];
  out->v[ 5] =  m->v[ 0]*m->v[10]*m->v[15] - m->v[ 0]*m->v[14]*m->v[11] - m->v[ 2]*m->v[ 8]*m->v[15] + m->v[ 2]*m->v[12]*m->v[11] + m->v[ 3]*m->v[ 8]*m->v[14] - m->v[ 3]*m->v[12]*m->v[10];
  out->v[ 6] = -m->v[ 0]*m->v[ 6]*m->v[15] + m->v[ 0]*m->v[14]*m->v[ 7] + m->v[ 2]*m->v[ 4]*m->v[15] - m->v[ 2]*m->v[12]*m->v[ 7] - m->v[ 3]*m->v[ 4]*m->v[14] + m->v[ 3]*m->v[12]*m->v[ 6];
  out->v[ 7] =  m->v[ 0]*m->v[ 6]*m->v[11] - m->v[ 0]*m->v[10]*m->v[ 7] - m->v[ 2]*m->v[ 4]*m->v[11] + m->v[ 2]*m->v[ 8]*m->v[ 7] + m->v[ 3]*m->v[ 4]*m->v[10] - m->v[ 3]*m->v[ 8]*m->v[ 6];

  out->v[ 8] =  m->v[ 4]*m->v[ 9]*m->v[15] - m->v[ 4]*m->v[13]*m->v[11] - m->v[ 5]*m->v[ 8]*m->v[15] + m->v[ 5]*m->v[12]*m->v[11] + m->v[ 7]*m->v[ 8]*m->v[13] - m->v[ 7]*m->v[12]*m->v[ 9];
  out->v[ 9] = -m->v[ 0]*m->v[ 9]*m->v[15] + m->v[ 0]*m->v[13]*m->v[11] + m->v[ 1]*m->v[ 8]*m->v[15] - m->v[ 1]*m->v[12]*m->v[11] - m->v[ 3]*m->v[ 8]*m->v[13] + m->v[ 3]*m->v[12]*m->v[ 9];
  out->v[10] =  m->v[ 0]*m->v[ 5]*m->v[15] - m->v[ 0]*m->v[13]*m->v[ 7] - m->v[ 1]*m->v[ 4]*m->v[15] + m->v[ 1]*m->v[12]*m->v[ 7] + m->v[ 3]*m->v[ 4]*m->v[13] - m->v[ 3]*m->v[12]*m->v[ 5];
  out->v[11] = -m->v[ 0]*m->v[ 5]*m->v[11] + m->v[ 0]*m->v[ 9]*m->v[ 7] + m->v[ 1]*m->v[ 4]*m->v[11] - m->v[ 1]*m->v[ 8]*m->v[ 7] - m->v[ 3]*m->v[ 4]*m->v[ 9] + m->v[ 3]*m->v[ 8]*m->v[ 5];

  out->v[12] = -m->v[ 4]*m->v[ 9]*m->v[14] + m->v[ 4]*m->v[13]*m->v[10] + m->v[ 5]*m->v[ 8]*m->v[14] - m->v[ 5]*m->v[12]*m->v[10] - m->v[ 6]*m->v[ 8]*m->v[13] + m->v[ 6]*m->v[12]*m->v[ 9];
  out->v[13] =  m->v[ 0]*m->v[ 9]*m->v[14] - m->v[ 0]*m->v[13]*m->v[10] - m->v[ 1]*m->v[ 8]*m->v[14] + m->v[ 1]*m->v[12]*m->v[10] + m->v[ 2]*m->v[ 8]*m->v[13] - m->v[ 2]*m->v[12]*m->v[ 9];
  out->v[14] = -m->v[ 0]*m->v[ 5]*m->v[14] + m->v[ 0]*m->v[13]*m->v[ 6] + m->v[ 1]*m->v[ 4]*m->v[14] - m->v[ 1]*m->v[12]*m->v[ 6] - m->v[ 2]*m->v[ 4]*m->v[13] + m->v[ 2]*m->v[12]*m->v[ 5];
  out->v[15] =  m->v[ 0]*m->v[ 5]*m->v[10] - m->v[ 0]*m->v[ 9]*m->v[ 6] - m->v[ 1]*m->v[ 4]*m->v[10] + m->v[ 1]*m->v[ 8]*m->v[ 6] + m->v[ 2]*m->v[ 4]*m->v[ 9] - m->v[ 2]*m->v[ 8]*m->v[ 5];

  const float det = 1.0f / (m->v[0]*out->v[0] + m->v[1]*out->v[4] + m->v[2]*out->v[8] + m->v[3]*out->v[12]);
  for (int i = 0; i < 16; i++) out->v[i] *= det;

  return out;
}

void mat4_viewport(mat4f *m, float x, float y, float w, float h, float znear, float zfar) {
  *m = mat4_zero;
  m->m[0][0] = w / 2.0f;
  m->m[0][3] = x + w / 2.0f;
  m->m[1][1] = h / -2.0f;
  m->m[1][3] = y + h / 2.0f;
  m->m[2][2] = (zfar - znear) / 2.f;
  m->m[2][3] = (zfar + znear) / 2.f;
  m->m[3][3] = 1.0f;
}
