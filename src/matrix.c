#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <x86intrin.h>

#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "error.h"
#include "state.h"
#include "matrix.h"

/* NOTE: matrices are stored in column major order, like in GL */

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

static mat4f mtx_stack_mv[MTX_STACK_SIZE_MV];
static mat4f mtx_stack_p[MTX_STACK_SIZE_P];
static mat4f mtx_stack_t[MTX_STACK_SIZE_T];

static struct mat4f_stack {
  mat4f *data;
  const int max;
  int ptr;
} mtx_stack[MTX_COUNT] = {
  { mtx_stack_mv, MTX_STACK_SIZE_MV },
  { mtx_stack_p,  MTX_STACK_SIZE_P  },
  { mtx_stack_t,  MTX_STACK_SIZE_T  },
};

/* pbgl internals */

void pbgl_mtx_reset(GLenum stack) {
  mtx_stack[stack].ptr = 0;
  pbgl.mtx[stack].mtx = mat4_identity;
}

mat4f *pbgl_mtx_peek(GLenum stack) {
  if (mtx_stack[stack].ptr)
    return &mtx_stack[stack].data[mtx_stack[stack].ptr - 1];
  return NULL;
}

GLint pbgl_mtx_stack_depth(GLenum stack) {
  return mtx_stack[stack].ptr;
}

/* GL FUNCTIONS BEGIN */

GL_API void glMatrixMode(GLenum mode) {
  switch (mode) {
    case GL_MODELVIEW:  pbgl.mtx_current = MTX_MODELVIEW; break;
    case GL_PROJECTION: pbgl.mtx_current = MTX_PROJECTION; break;
    case GL_TEXTURE:    pbgl.mtx_current = MTX_TEXTURE; break;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      break;
  }
}

GL_API void glLoadMatrixf(const GLfloat *m) {
  memcpy(pbgl.mtx[pbgl.mtx_current].mtx.v, m, 16 * sizeof(float));
  pbgl.mtx[pbgl.mtx_current].identity = GL_FALSE;
  pbgl.state_dirty = pbgl.mtx_any_dirty = pbgl.mtx[pbgl.mtx_current].dirty = GL_TRUE;
}

GL_API void glLoadMatrixd(const GLdouble *m) {
  // we internally operate on floats
  for (int i = 0; i < 16; ++i)
    pbgl.mtx[pbgl.mtx_current].mtx.v[i] = (float)m[i];
  pbgl.mtx[pbgl.mtx_current].identity = GL_FALSE;
  pbgl.state_dirty = pbgl.mtx_any_dirty = pbgl.mtx[pbgl.mtx_current].dirty = GL_TRUE;
}

GL_API void glLoadIdentity(void) {
  pbgl.mtx[pbgl.mtx_current].mtx = mat4_identity;
  pbgl.mtx[pbgl.mtx_current].identity = GL_TRUE;
  pbgl.state_dirty = pbgl.mtx_any_dirty = pbgl.mtx[pbgl.mtx_current].dirty = GL_TRUE;
}

GL_API void glLoadTransposeMatrixf(const GLfloat *m) {
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      pbgl.mtx[pbgl.mtx_current].mtx.m[i][j] = m[j * 4 + i];
  pbgl.mtx[pbgl.mtx_current].identity = GL_FALSE;
  pbgl.state_dirty = pbgl.mtx_any_dirty = pbgl.mtx[pbgl.mtx_current].dirty = GL_TRUE;
}

GL_API void glLoadTransposeMatrixd(const GLdouble *m) {
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      pbgl.mtx[pbgl.mtx_current].mtx.m[i][j] = (float)m[j * 4 + i];
  pbgl.mtx[pbgl.mtx_current].identity = GL_FALSE;
  pbgl.state_dirty = pbgl.mtx_any_dirty = pbgl.mtx[pbgl.mtx_current].dirty = GL_TRUE;
}

GL_API void glMultMatrixf(const GLfloat *m) {
  mat4_mul(&pbgl.mtx[pbgl.mtx_current].mtx, &pbgl.mtx[pbgl.mtx_current].mtx, (const mat4f *)m);
  pbgl.mtx[pbgl.mtx_current].identity = GL_FALSE;
  pbgl.state_dirty = pbgl.mtx_any_dirty = pbgl.mtx[pbgl.mtx_current].dirty = GL_TRUE;
}

GL_API void glMultMatrixd(const GLdouble *m) {
  mat4f tmp;
  for (int i = 0; i < 16; ++i)
    tmp.v[i] = (float)m[i];
  mat4_mul(&pbgl.mtx[pbgl.mtx_current].mtx, &pbgl.mtx[pbgl.mtx_current].mtx, &tmp);
  pbgl.mtx[pbgl.mtx_current].identity = GL_FALSE;
  pbgl.state_dirty = pbgl.mtx_any_dirty = pbgl.mtx[pbgl.mtx_current].dirty = GL_TRUE;
}

GL_API void glMultTransposeMatrixf(const GLfloat *m) {
  mat4f tmp;
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      tmp.m[i][j] = m[j * 4 + i];
  mat4_mul(&pbgl.mtx[pbgl.mtx_current].mtx, &pbgl.mtx[pbgl.mtx_current].mtx, &tmp);
  pbgl.mtx[pbgl.mtx_current].identity = GL_FALSE;
  pbgl.state_dirty = pbgl.mtx_any_dirty = pbgl.mtx[pbgl.mtx_current].dirty = GL_TRUE;
}

GL_API void glMultTransposeMatrixd(const GLdouble *m) {
  mat4f tmp;
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      tmp.m[i][j] = (float)m[j * 4 + i];
  mat4_mul(&pbgl.mtx[pbgl.mtx_current].mtx, &pbgl.mtx[pbgl.mtx_current].mtx, &tmp);
  pbgl.mtx[pbgl.mtx_current].identity = GL_FALSE;
  pbgl.state_dirty = pbgl.mtx_any_dirty = pbgl.mtx[pbgl.mtx_current].dirty = GL_TRUE;
}

GL_API void glPushMatrix(void) {
  struct mat4f_stack *s = mtx_stack + pbgl.mtx_current;
  if (s->ptr >= s->max)
    pbgl_set_error(GL_STACK_OVERFLOW);
  else
    s->data[s->ptr++] = pbgl.mtx[pbgl.mtx_current].mtx;
}

GL_API void glPopMatrix(void) {
  struct mat4f_stack *s = mtx_stack + pbgl.mtx_current;
  if (s->ptr <= 0)
    pbgl_set_error(GL_STACK_OVERFLOW);
  else
    pbgl.mtx[pbgl.mtx_current].mtx = s->data[s->ptr--];
  pbgl.mtx[pbgl.mtx_current].identity = GL_FALSE;
  pbgl.state_dirty = pbgl.mtx_any_dirty = pbgl.mtx[pbgl.mtx_current].dirty = GL_TRUE;
}

GL_API void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
  mat4f tmp = mat4_identity;
  tmp.m[0][3] = x;
  tmp.m[1][3] = y;
  tmp.m[2][3] = z;
  glMultMatrixf(tmp.v);
}

GL_API void glTranslated(GLdouble x, GLdouble y, GLdouble z) {
  glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
}

GL_API void glScalef(GLfloat x, GLfloat y, GLfloat z) {
  mat4f tmp = {{ 0.f }};
  tmp.m[0][0] = x;
  tmp.m[1][1] = y;
  tmp.m[2][2] = z;
  tmp.m[3][3] = 1.f;
  glMultMatrixf(tmp.v);
}

GL_API void glScaled(GLdouble x, GLdouble y, GLdouble z) {
  glScalef((GLfloat)x, (GLfloat)y, (GLfloat)z);
}

GL_API void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
  mat4f tmp = {{ 0.f }};

  if ((x == 0.f && y == 0.f && z == 0.f) || (angle == 0.f))
    return;

  // normalize axis vector
  const GLfloat len = 1.f / sqrtf(x * x + y * y + z * z);
  x *= len; y *= len; z *= len;

  // angle is passed in in degrees
  angle *= M_PI / 180.f;
  const GLfloat s = sinf(angle);
  const GLfloat c = cosf(angle);
  const GLfloat ic = 1.f - c;
  // might as well precalc these too
  const GLfloat xic = x * ic, yic = y * ic, zic = z * ic;

  tmp.m[0][0] = x * xic + c;
  tmp.m[0][1] = y * xic + z * s;
  tmp.m[0][2] = z * xic - y * s;

  tmp.m[1][0] = x * yic - z * s;
  tmp.m[1][1] = y * yic + c;
  tmp.m[1][2] = z * yic + x * s;

  tmp.m[2][0] = x * zic + y * s;
  tmp.m[2][1] = y * zic - x * s;
  tmp.m[2][2] = z * zic + c;

  tmp.m[3][3] = 1.f;

  glMultMatrixf(tmp.v);
}

GL_API void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {
  glRotatef((GLfloat)angle, (GLfloat)x, (GLfloat)y, (GLfloat)z);
}

GL_API void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble znear, GLdouble zfar) {
  mat4f tmp = {{ 0.f }};

  tmp.m[0][0] = 2.0 / (right - left);
  tmp.m[0][3] = (left + right) / (left - right);

  tmp.m[1][1] = 2.0 / (top - bottom);
  tmp.m[1][3] = (top + bottom) / (bottom - top);

  tmp.m[2][2] = 1.0 / (zfar - znear);
  tmp.m[2][3] = znear / (znear - zfar);

  tmp.m[3][3] = 1.f;

  glMultMatrixf(tmp.v);
}

GL_API void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble znear, GLdouble zfar) {
  mat4f tmp = {{ 0.f }};

  tmp.m[0][0] = 2.0 * znear / (right - left);
  tmp.m[2][0] = (left + right) / (right - left);

  tmp.m[1][1] = 2.0 * znear / (top - bottom);
  tmp.m[2][1] = (top + bottom) / (top - bottom);

  tmp.m[2][2] = -(zfar + znear) / (zfar - znear);
  tmp.m[3][2] = -1.f;

  tmp.m[2][3] = -2.0 * znear * zfar / (zfar - znear);

  glMultMatrixf(tmp.v);
}

GL_API void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble znear, GLdouble zfar) {
  const GLdouble angle = fovy / 360.0 * M_PI;
  const GLdouble fh = (sin(angle) / cos(angle)) * znear; // FIXME: replace with tan() when it's fixed
  const GLdouble fw = fh * aspect;
  glFrustum(-fw, fw, -fh, fh, znear, zfar);
}
