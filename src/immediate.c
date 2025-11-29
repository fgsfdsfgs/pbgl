#include <stdlib.h>
#include <limits.h>
#include <pbkit/pbkit.h>

#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "error.h"
#include "texture.h"
#include "array.h"
#include "misc.h"
#include "push.h"
#include "state.h"
#include "immediate.h"

static inline GLuint *push_attr_vec4f(GLuint *p, GLuint array, vec4f v) {
    p = push_command(p, NV097_SET_VERTEX_DATA4F_M + array * 4 * 4, 4);
    p = push_float(p, v.x);
    p = push_float(p, v.y);
    p = push_float(p, v.z);
    p = push_float(p, v.w);
    return p;
}

static inline GLuint *push_attr_vec3f(GLuint *p, GLuint array, vec3f v) {
    p = push_command(p, NV097_SET_VERTEX_DATA4F_M + array * 4 * 4, 4);
    p = push_float(p, v.x);
    p = push_float(p, v.y);
    p = push_float(p, v.z);
    p = push_float(p, 0.f);
    return p;
}

/* GL FUNCTIONS BEGIN */

GL_API void glBegin(GLenum prim) {
  if (pbgl.imm.active) {
    // already in begin .. end
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  // NV2A prim type values are the same as GL prim types,
  // but with 1 added to account for OP_END = 0, and they go from 1 to A
  if (prim >= 0xA) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  pbgl.imm.prim = prim + 1;
  pbgl.imm.active = GL_TRUE;
  pbgl.imm.num = 0;
  pbgl.imm.flags = 0;

  pbgl_state_flush();

  GLuint *p = pb_begin();
  p = push_command_parameter(p, NV097_SET_BEGIN_END, pbgl.imm.prim);
  pb_end(p);
}

GL_API void glEnd(void) {
  if (!pbgl.imm.active) {
    // not in begin .. end
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  GLuint *p = pb_begin();
  p = push_command_parameter(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
  pb_end(p);

  pbgl.imm.active = GL_FALSE;
  pbgl.imm.ptr = NULL;
}

GL_API void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
  pbgl.imm.color_flag = GL_TRUE;
  pbgl.varray[VARR_COLOR1].value.r = r;
  pbgl.varray[VARR_COLOR1].value.g = g;
  pbgl.varray[VARR_COLOR1].value.b = b;
  pbgl.varray[VARR_COLOR1].value.a = a;
  pbgl.varray[VARR_COLOR1].dirty = GL_TRUE;
}

GL_API void glColor4fv(const GLfloat *v) {
  glColor4f(v[0], v[1], v[2], v[3]);
}

GL_API void glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
  glColor4f((GLfloat)r / 255.f, (GLfloat)g / 255.f, (GLfloat)b / 255.f, (GLfloat)a / 255.f);
}

GL_API void glColor4ubv(const GLubyte *v) {
  glColor4ub(v[0], v[1], v[2], v[3]);
}

GL_API void glColor3f(GLfloat r, GLfloat g, GLfloat b) {
  glColor4f(r, g, b, 1.f);
}

GL_API void glColor3fv(const GLfloat *v) {
  glColor4f(v[0], v[1], v[2], 1.f);
}

GL_API void glColor3ub(GLubyte r, GLubyte g, GLubyte b) {
  glColor4ub(r, g, b, 255);
}

GL_API void glColor3ubv(const GLubyte *v) {
  glColor4ub(v[0], v[1], v[2], 255);
}

GL_API void glSecondaryColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
  pbgl.imm.color2_flag = GL_TRUE;
  pbgl.varray[VARR_COLOR2].value.r = r;
  pbgl.varray[VARR_COLOR2].value.g = g;
  pbgl.varray[VARR_COLOR2].value.b = b;
  pbgl.varray[VARR_COLOR2].value.a = a;
  pbgl.varray[VARR_COLOR2].dirty = GL_TRUE;
}

GL_API void glSecondaryColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
  glSecondaryColor4f((GLfloat)r / 255.f, (GLfloat)g / 255.f, (GLfloat)b / 255.f, (GLfloat)a / 255.f);
}

GL_API void glSecondaryColor3f(GLfloat r, GLfloat g, GLfloat b) {
  glSecondaryColor4f(r, g, b, 1.f);
}

GL_API void glSecondaryColor3ub(GLubyte r, GLubyte g, GLubyte b) {
  glSecondaryColor4ub(r, g, b, 255);
}

GL_API void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
  // according to spec, glTexCoord is equivalent to this with multitexturing support
  pbgl.imm.texcoord_flag = GL_TRUE;
  pbgl.tex[0].varray.value.x = s;
  pbgl.tex[0].varray.value.y = t;
  pbgl.tex[0].varray.value.z = r;
  pbgl.tex[0].varray.value.w = q;
  pbgl.tex[0].varray.dirty = GL_TRUE;
}

GL_API void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r) {
  glTexCoord4f(s, t, r, 1.f);
}

GL_API void glTexCoord3fv(const GLfloat *v) {
  glTexCoord4f(v[0], v[1], v[2], 1.f);
}

GL_API void glTexCoord3i(GLint s, GLint t, GLint r) {
  glTexCoord4f((GLfloat)s, (GLfloat)t, (GLfloat)r, 1.f);
}

GL_API void glTexCoord3iv(const GLint *v) {
  glTexCoord4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.f);
}

GL_API void glTexCoord2f(GLfloat s, GLfloat t) {
  glTexCoord4f(s, t, 0.f, 1.f);
}

GL_API void glTexCoord2fv(const GLfloat *v) {
  glTexCoord4f(v[0], v[1], 0.f, 1.f);
}

GL_API void glTexCoord2i(GLint s, GLint t) {
  glTexCoord4f((GLfloat)s, (GLfloat)t, 0.f, 1.f);
}

GL_API void glTexCoord2iv(const GLint *v) {
  glTexCoord4f((GLfloat)v[0], (GLfloat)v[1], 0.f, 1.f);
}

GL_API void glMultiTexCoord4f(GLenum unit, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
  unit -= GL_TEXTURE0;
  pbgl.imm.texcoord_flag = GL_TRUE;
  pbgl.tex[unit].varray.value.x = s;
  pbgl.tex[unit].varray.value.y = t;
  pbgl.tex[unit].varray.value.z = r;
  pbgl.tex[unit].varray.value.w = q;
  pbgl.tex[unit].varray.dirty = GL_TRUE;
}

GL_API void glMultiTexCoord3f(GLenum unit, GLfloat s, GLfloat t, GLfloat r) {
  glMultiTexCoord4f(unit, s, t, r, 1.f);
}

GL_API void glMultiTexCoord2f(GLenum unit, GLfloat s, GLfloat t) {
  glMultiTexCoord4f(unit, s, t, 0.f, 1.f);
}

GL_API void glNormal3f(GLfloat x, GLfloat y, GLfloat z) {
  pbgl.imm.normal_flag = GL_TRUE;
  pbgl.varray[VARR_NORMAL].value.x = x;
  pbgl.varray[VARR_NORMAL].value.y = y;
  pbgl.varray[VARR_NORMAL].value.z = z;
  pbgl.varray[VARR_NORMAL].dirty = GL_TRUE;
}

GL_API void glNormal3fv(const GLfloat *v) {
  glNormal3f(v[0], v[1], v[2]);
}

GL_API void glNormal3d(GLdouble x, GLdouble y, GLdouble z) {
  glNormal3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
}

GL_API void glNormal3dv(const GLdouble *v) {
  glNormal3f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

GL_API void glFogCoordf(GLfloat x) {
  pbgl.varray[VARR_FOG].value.x = x;
  pbgl.varray[VARR_FOG].value.y = x;
  pbgl.varray[VARR_FOG].value.z = x;
  pbgl.varray[VARR_FOG].value.w = x;
  pbgl.imm.fogcoord_flag = GL_TRUE;
}

GL_API void glFogCoordd(GLdouble x) {
  glFogCoordf((GLfloat)x);
}

GL_API void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
  GLuint *p = pb_begin();

  p = push_attr_vec4f(p, NV2A_VERTEX_ATTR_DIFFUSE, pbgl.varray[VARR_COLOR1].value);

  if (pbgl.imm.color2_flag)
    p = push_attr_vec4f(p, NV2A_VERTEX_ATTR_SPECULAR, pbgl.varray[VARR_COLOR2].value);

  if (pbgl.imm.normal_flag)
    p = push_attr_vec3f(p, NV2A_VERTEX_ATTR_NORMAL, pbgl.varray[VARR_NORMAL].value.vec3);

  if (pbgl.imm.fogcoord_flag)
    p = push_attr_vec4f(p, NV2A_VERTEX_ATTR_FOG, pbgl.varray[VARR_FOG].value);

  if (pbgl.imm.texcoord_flag) {
    // load different texcoords for each texture unit that is enabled
    for (int i = 0; i < TEXUNIT_COUNT; ++i)
      if (pbgl.tex[i].enabled)
        p = push_attr_vec4f(p, NV2A_VERTEX_ATTR_TEXTURE0 + i, pbgl.tex[i].varray.value);
  }

  p = push_command(p, NV097_SET_VERTEX4F, 4);
  p = push_float(p, x);
  p = push_float(p, y);
  p = push_float(p, z);
  p = push_float(p, w);

  pb_end(p);

  ++pbgl.imm.num;
}

GL_API void glVertex4fv(const GLfloat *v) {
  glVertex4f(v[0], v[1], v[2], v[3]);
}

GL_API void glVertex4i(GLint x, GLint y, GLint z, GLint w) {
  glVertex4f((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
}

GL_API void glVertex4iv(const GLint *v) {
  glVertex4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3]);
}

GL_API void glVertex3f(GLfloat x, GLfloat y, GLfloat z) {
  glVertex4f(x, y, z, 1.f);
}

GL_API void glVertex3fv(const GLfloat *v) {
  glVertex4f(v[0], v[1], v[2], 1.f);
}

GL_API void glVertex3i(GLint x, GLint y, GLint z) {
  glVertex4f((GLfloat)x, (GLfloat)y, (GLfloat)z, 1.f);
}

GL_API void glVertex3iv(const GLint *v) {
  glVertex4f((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.f);
}

GL_API void glVertex2f(GLfloat x, GLfloat y) {
  glVertex4f(x, y, 0.f, 1.f);
}

GL_API void glVertex2fv(const GLfloat *v) {
  glVertex4f(v[0], v[1], 0.f, 1.f);
}

GL_API void glVertex2i(GLint x, GLint y) {
  glVertex4f((GLfloat)x, (GLfloat)y, 0.f, 1.f);
}

GL_API void glVertex2iv(const GLint *v) {
  glVertex4f((GLfloat)v[0], (GLfloat)v[1], 0.f, 1.f);
}

GL_API void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
  // by spec glRect is "exactly equivalent" to this
  glBegin(GL_POLYGON);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
  glEnd();
}

GL_API void glRecti(GLint x1, GLint y1, GLint x2, GLint y2) {
  glRectf((GLfloat)x1, (GLfloat)y1, (GLfloat)x2, (GLfloat)y2);
}

GL_API void glRectfv(const GLfloat *v1, const GLfloat *v2) {
  glRectf(v1[0], v1[1], v2[0], v2[1]);
}

GL_API void glRectiv(const GLint *v1, const GLint *v2) {
  glRectf((GLfloat)v1[0], (GLfloat)v1[1], (GLfloat)v2[0], (GLfloat)v2[1]);
}
