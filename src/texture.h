#ifndef _PBGL_TEXTURE_H
#define _PBGL_TEXTURE_H

#include "GL/gl.h"
#include "GL/glext.h"

#define TEXUNIT_COUNT 4
#define TEX_ALIGN 16
#define TEX_MAX_SIZE 4096
#define TEX_MAX_MIPS 12 // log2(4096)

typedef struct {
  GLubyte *data;
  GLuint width;
  GLuint height;
  GLuint size;
  GLuint pitch;
} mipmap_t;

typedef struct {
  struct {
    GLuint format;     // all nv format flags
    GLuint filter;     // combined min/mag filter f;ags
    GLuint wrap;       // combined s/t addressing mode flags
    GLuint addr;       // data & 0x7FFFFFFF
    GLboolean linear;  // is the texture linear?
    GLboolean aligned; // is size 64-byte aligned?
  } nv;

  struct {
    GLenum type;
    GLenum format;
    GLenum baseformat;
    GLenum min_filter;
    GLenum mag_filter;
    GLenum wrap_s;
    GLenum wrap_t;
    GLenum wrap_r;
    GLboolean border;
    GLboolean gen_mipmap;
  } gl;

  GLboolean used;
  GLboolean allocated;
  GLboolean bound;
  GLenum target;
  GLuint dimensions;
  GLuint width;
  GLuint height;
  GLuint depth;

  GLuint size;    // total data size
  GLuint pitch;   // stride for one line
  GLuint zpitch;  // stride for one z level of 3d texture
  GLuint bytespp; // bytes per pixel

  GLboolean mipmap;
  GLuint mipmax;
  GLuint mipcount;
  mipmap_t mips[TEX_MAX_MIPS];

  GLubyte *data;
} texture_t;

GLboolean pbgl_tex_init(void);
void pbgl_tex_free(void);

#endif // _PBGL_TEXTURE_H
