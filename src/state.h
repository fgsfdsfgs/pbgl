#ifndef _PBGL_STATE_H
#define _PBGL_STATE_H

#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "matrix.h"
#include "light.h"
#include "texture.h"
#include "array.h"

typedef union {
  struct {
    GLuint alpha_test   : 1; // 0
    GLuint depth_test   : 1; // 1
    GLuint stencil_test : 1; // 2
    GLuint scissor_test : 1; // 3
    GLuint blend        : 1; // 4
    GLuint cull_face    : 1; // 5
    GLuint dither       : 1; // 6
    GLuint fog          : 1; // 7
    GLuint lighting     : 1; // 8
    GLuint poly_offset  : 1; // 9
    GLuint texture_1d   : 1; // 10
    GLuint texture_2d   : 1; // 11
    GLuint texture_3d   : 1; // 12
    GLuint texture_cube : 1; // 13
    GLuint texgen_q     : 1; // 14
    GLuint texgen_r     : 1; // 15
    GLuint texgen_s     : 1; // 16
    GLuint texgen_t     : 1; // 17
  };
  GLuint word;
} server_flags_t;

typedef struct {
  GLboolean enabled;
  GLenum type;
  GLsizei size;
  GLsizei stride;
  const GLubyte *data;
} varray_state_t;

typedef struct {
  GLboolean dirty;
  GLenum func;
  GLenum op_sfail;
  GLenum op_zfail;
  GLenum op_zpass;
  GLint ref;
  GLuint funcmask;
  GLuint writemask;
} stencil_state_t;

typedef struct {
  GLboolean dirty;
  GLenum func;
  GLboolean writemask;
} depth_state_t;

typedef struct {
  GLboolean dirty;
  GLclampf factor;
  GLclampf units;
} polyofs_state_t;

typedef struct {
  GLboolean dirty;
  GLenum func;
  GLubyte ref;
} alpha_state_t;

typedef struct {
  GLboolean dirty;
  GLenum src;
  GLenum dst;
  GLenum eqn;
  GLuint color;
} blend_state_t;

typedef struct {
  GLboolean dirty;
  GLuint value;
} color_mask_t;

typedef struct {
  GLboolean dirty;
  GLboolean identity;
  mat4f mtx;
} matrix_state_t;

typedef struct {
  GLboolean dirty;
  GLboolean enabled;
  vec4f ambient;
  vec4f diffuse;
  vec4f specular;
  vec4f pos;
  GLfloat factor[LFACTOR_COUNT];
} light_state_t;

typedef struct {
  GLboolean dirty;
  GLboolean local;
  GLboolean twosided;
  vec4f ambient;
} light_model_state_t;

typedef struct {
  GLenum dirty;
  GLenum mode;
  GLenum combine_rgb;
  GLenum combine_a;
  GLfloat scale_rgb;
  GLfloat scale_a;
  GLenum src_rgb[3];
  GLenum src_a[3];
  GLenum operand_rgb[3];
  GLenum operand_a[3];
  GLuint color;
  GLuint nvshader;
} texenv_state_t;

typedef struct {
  GLboolean enabled;
  GLboolean dirty;
  vec4f texcoord;
  varray_state_t varray; // texcoord array
  texture_t *tex; // TODO: need separate targets for _1D, _2D, _3D
} texunit_state_t;

typedef struct {
  GLboolean dirty;
  GLenum frontface;
  GLenum cullface;
} cull_state_t;

typedef struct {
  GLboolean dirty;
  GLenum mode;
  GLfloat density;
  GLfloat start;
  GLfloat end;
  GLuint color;
} fog_state_t;

typedef struct {
  GLboolean dirty;
  GLint x;
  GLint y;
  GLint w;
  GLint h;
} rect_state_t;

typedef struct {
  GLboolean dirty;
  GLint x;
  GLint y;
  GLint w;
  GLint h;
  GLclampf znear;
  GLclampf zfar;
  mat4f mtx;
} viewport_state_t;

typedef struct {
  GLboolean active;
  GLenum prim;
  GLuint num;
  GLuint *ptr;
  vec4f color;
  vec4f color2;
  vec4f texcoord;
  vec3f normal;
  GLfloat fogcoord;
  union {
    struct {
      GLuint color_flag : 1;
      GLuint color2_flag : 1;
      GLuint texcoord_flag : 1;
      GLuint normal_flag : 1;
      GLuint fogcoord_flag : 1;
      GLuint multitexcoord_flag : 1;
    };
    GLuint flags;
  };
} immediate_state_t;

typedef struct {
  GLboolean active;
  GLenum error;
  GLboolean state_dirty;

  server_flags_t flags;

  viewport_state_t view;
  rect_state_t scissor;
  alpha_state_t alpha;
  blend_state_t blend;
  depth_state_t depth;
  stencil_state_t stencil;
  polyofs_state_t polyofs;
  color_mask_t colormask;
  cull_state_t cullface;
  light_model_state_t lightmodel;
  fog_state_t fog;

  immediate_state_t imm;
  varray_state_t varray[VARR_COUNT];

  texunit_state_t tex[TEXUNIT_COUNT];
  GLboolean tex_any_dirty;

  texenv_state_t texenv[TEXUNIT_COUNT];
  GLboolean texenv_dirty;

  light_state_t light[LIGHT_COUNT];
  GLboolean light_any_dirty;

  matrix_state_t mtx[MTX_COUNT];
  GLenum mtx_current;
  GLboolean mtx_any_dirty;

  GLint active_tex_sv;
  GLint active_tex_cl;

  GLuint clear_color;
  GLuint clear_zstencil;

  GLuint fb_width;
  GLuint fb_height;

  GLint last_swap;
  GLint swap_interval;
} gl_state_t;

extern gl_state_t pbgl;

void pbgl_state_init(void);
GLboolean pbgl_state_flush(void);

#endif // _PBGL_STATE_H
