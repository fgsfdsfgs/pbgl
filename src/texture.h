#ifndef _PBGL_TEXTURE_H
#define _PBGL_TEXTURE_H

#include "GL/gl.h"
#include "GL/glext.h"

#define TEXUNIT_COUNT 4
#define TEX_MAX_SIZE 4096
#define TEX_MAX_MIPS 12 // log2(4096)

#define PAL_ALIGN 64
#define PAL_BYTESPP 4 // internally palettes are always RGBA8888
#define PAL_MAX_WIDTH 256

typedef struct {
  GLubyte *data;
  GLuint width;
  GLuint height;
  GLuint size;
  GLuint pitch;
} mipmap_t;

typedef struct {
  struct {
    GLuint format;         // all nv format flags
    GLuint palette;        // palette format flags
    GLuint filter;         // combined min/mag filter f;ags
    GLuint wrap;           // combined s/t addressing mode flags
    GLuint addr;           // data & 0x7FFFFFFF
    GLboolean linear : 1;  // is the texture linear?
    GLboolean aligned : 1; // is size 64-byte aligned?
  } nv;

  struct {
    GLenum basetarget;
    GLenum type;
    GLenum format;
    GLenum baseformat;
    GLenum min_filter;
    GLenum mag_filter;
    GLenum wrap_s;
    GLenum wrap_t;
    GLenum wrap_r;
    GLboolean border : 1;
    GLboolean gen_mipmap : 1;
  } gl;

  struct {
    const GLubyte *source;
    GLubyte *data;
    GLenum baseformat;
    GLenum extformat;
    GLushort intbytespp; // requested by client; internally all palettes are 4 bytes
    GLushort extbytespp;
    GLushort width;
  } palette;

  GLboolean used : 1;
  GLboolean allocated : 1;
  GLboolean bound : 1;
  GLboolean mipmap : 1;
  GLboolean mipmap_expected : 1;

  GLenum target;
  GLuint dimensions;
  GLuint width;
  GLuint height;
  GLuint depth;

  GLuint size;    // texture data size (all mips)
  GLuint total;   // size + palette data size
  GLuint pitch;   // stride for one line
  GLuint zpitch;  // stride for one z level of 3d texture
  GLuint bytespp; // bytes per pixel

  GLuint mipmax;
  GLuint mipcount;
  mipmap_t mips[TEX_MAX_MIPS];

  GLubyte *data;
} texture_t;

GLboolean pbgl_tex_init(void);
void pbgl_tex_free(void);

GLuint pbgl_tex_get_cap(void);
GLuint pbgl_tex_get_num(void);
GLuint pbgl_tex_get_used_memory(void);

#endif // _PBGL_TEXTURE_H
