#include <stdlib.h>
#include <limits.h>
#include <windows.h>
#include <pbkit/pbkit.h>
#include <x86intrin.h>

#define GL_GLEXT_PROTOTYPES
#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "error.h"
#include "state.h"
#include "misc.h"
#include "pbgl.h"
#include "memory.h"
#include "swizzle.h"
#include "texture.h"

#define PAL_DMA_A 0
#define PAL_DMA_B 1

#define TEX_DMA_A 1
#define TEX_DMA_B 2

#define TEX_ALLOC_STEP 256

#define NV_PGRAPH_TEXADDRESS0_ADDRU_WRAP          1
#define NV_PGRAPH_TEXADDRESS0_ADDRU_MIRROR        2
#define NV_PGRAPH_TEXADDRESS0_ADDRU_CLAMP_TO_EDGE 3
#define NV_PGRAPH_TEXADDRESS0_ADDRU_BORDER        4
#define NV_PGRAPH_TEXADDRESS0_ADDRU_CLAMP_OGL     5

#define NV_PGRAPH_TEXFILTER0_MIN_BOX_LOD0            1
#define NV_PGRAPH_TEXFILTER0_MIN_TENT_LOD0           2
#define NV_PGRAPH_TEXFILTER0_MIN_BOX_NEARESTLOD      3
#define NV_PGRAPH_TEXFILTER0_MIN_TENT_NEARESTLOD     4
#define NV_PGRAPH_TEXFILTER0_MIN_BOX_TENT_LOD        5
#define NV_PGRAPH_TEXFILTER0_MIN_TENT_TENT_LOD       6
#define NV_PGRAPH_TEXFILTER0_MIN_CONVOLUTION_2D_LOD0 7

#define NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL          0x0000E000
#define NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL_QUINCUNX 1
#define NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL_GAUSSIAN 2

static texture_t *textures = NULL;
static GLuint tex_count = 0;
static GLuint tex_cap = 0;

static inline GLuint wrap_gl_to_nv(const GLenum glwrap) {
  switch (glwrap) {
    case GL_CLAMP:           return NV_PGRAPH_TEXADDRESS0_ADDRU_CLAMP_OGL;
    case GL_CLAMP_TO_EDGE:   return NV_PGRAPH_TEXADDRESS0_ADDRU_CLAMP_TO_EDGE;
    case GL_CLAMP_TO_BORDER: return NV_PGRAPH_TEXADDRESS0_ADDRU_BORDER;
    case GL_REPEAT:          return NV_PGRAPH_TEXADDRESS0_ADDRU_WRAP;
    case GL_MIRRORED_REPEAT: return NV_PGRAPH_TEXADDRESS0_ADDRU_MIRROR;
    default:                 return 0;
  }
}

static inline GLuint filter_gl_to_nv_min(const GLenum glfilter) {
  switch (glfilter) {
    case GL_NEAREST:                return NV_PGRAPH_TEXFILTER0_MIN_BOX_LOD0;
    case GL_LINEAR:                 return NV_PGRAPH_TEXFILTER0_MIN_TENT_LOD0;
    case GL_NEAREST_MIPMAP_NEAREST: return NV_PGRAPH_TEXFILTER0_MIN_BOX_NEARESTLOD;
    case GL_LINEAR_MIPMAP_NEAREST:  return NV_PGRAPH_TEXFILTER0_MIN_TENT_NEARESTLOD;
    case GL_NEAREST_MIPMAP_LINEAR:  return NV_PGRAPH_TEXFILTER0_MIN_BOX_TENT_LOD;
    case GL_LINEAR_MIPMAP_LINEAR:   return NV_PGRAPH_TEXFILTER0_MIN_TENT_TENT_LOD;
    default:                        return 0;
  }
}

static inline GLuint filter_gl_to_nv_mag(const GLenum glfilter) {
  switch (glfilter) {
    case GL_NEAREST: return NV_PGRAPH_TEXFILTER0_MIN_BOX_LOD0;
    case GL_LINEAR:  return NV_PGRAPH_TEXFILTER0_MIN_TENT_LOD0;
    default:         return 0;
  }
}

static inline GLuint intfmt_gl_to_nv(const GLenum glformat) {
  switch (glformat) {
    case GL_ALPHA:
    case GL_ALPHA8:            return NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A8;

    // FIXME: is this right? what even is Y?
    //        does GL_INTENSITY also map to Y?
    case 1: // legacy alias for GL_LUMINANCE
    case GL_LUMINANCE:
    case GL_LUMINANCE8:        return NV097_SET_TEXTURE_FORMAT_COLOR_SZ_Y8;

    case 2: // legacy alias for GL_LUMINANCE_ALPHA
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE8_ALPHA8: return NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A8Y8;

    case 3: // legacy alias for GL_RGB
    case GL_RGB:
    case GL_RGB8:              return NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A8B8G8R8;

    case 4: // legacy alias for GL_RGBA
    case GL_RGBA:
    case GL_RGBA8:             return NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A8B8G8R8;

    case GL_RGBA4:             return NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A4R4G4B4;
    case GL_RGB5_A1:           return NV097_SET_TEXTURE_FORMAT_COLOR_SZ_A1R5G5B5;
    case GL_RGB5:              return NV097_SET_TEXTURE_FORMAT_COLOR_SZ_X1R5G5B5;

    case GL_COLOR_INDEX:
    case GL_COLOR_INDEX8_EXT:  return NV097_SET_TEXTURE_FORMAT_COLOR_SZ_I8_A8R8G8B8;

    default:                   return 0;
  }
}

static inline GLuint intfmt_bytespp(const GLenum glformat) {
  switch (glformat) {
    case 1: // legacy alias for GL_LUMINANCE
    case GL_ALPHA:
    case GL_ALPHA8:
    case GL_LUMINANCE:
    case GL_LUMINANCE8:
    case GL_COLOR_INDEX:
    case GL_COLOR_INDEX8_EXT:
      return 1;

    case 2: // legacy alias for GL_LUMINANCE_ALPHA
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE8_ALPHA8:
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_RGB5:
      return 2;

    case 3: // legacy alias for GL_RGB
    case GL_RGB:
    case GL_RGB8:
      return 4; // NOTE: pixels are still 4 bytes each, A is just unused

    case 4: // legacy alias for GL_RGBA
    case GL_RGBA:
    case GL_RGBA8:
      return 4;

    default: // unknown format
      return 0;
  }
}

static inline GLenum intfmt_basefmt(const GLenum glformat) {
  switch (glformat) {
    case GL_ALPHA:
    case GL_ALPHA8:
      return GL_ALPHA;

    case 1: // legacy alias for GL_LUMINANCE
    case GL_LUMINANCE:
    case GL_LUMINANCE8:
      return GL_LUMINANCE;

    case 2: // legacy alias for GL_LUMINANCE_ALPHA
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE8_ALPHA8:
      return GL_LUMINANCE_ALPHA;

    case 3: // legacy alias for GL_RGB
    case GL_RGB:
    case GL_RGB8:
    case GL_RGB5:
      return GL_RGB;

    case 4: // legacy alias for GL_RGBA
    case GL_RGBA:
    case GL_RGBA8:
    case GL_RGBA4:
    case GL_RGB5_A1:
      return GL_RGBA;

    case GL_COLOR_INDEX8_EXT:
      return GL_COLOR_INDEX;

    default: // unknown format
      return 0;
  }
}

static inline GLuint extfmt_bytespp(const GLenum glformat, const GLuint type, GLboolean *reverse) {
  switch (glformat) {
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_LUMINANCE:
    case GL_COLOR_INDEX:
      return (type == GL_UNSIGNED_BYTE) ? 1 : 0;

    case GL_LUMINANCE_ALPHA:
      return (type == GL_UNSIGNED_BYTE) ? 2 : 0;

    // case GL_BGR:
    //   if (reverse) *reverse = GL_TRUE;
    //   /* fallthrough */
    case GL_RGB:
      return (type == GL_UNSIGNED_BYTE) ? 3 : 0;

    // case GL_BGRA:
    //   if (reverse) *reverse = GL_TRUE;
    //   /* fallthrough */
    case GL_RGBA:
      switch (type) {
        case GL_UNSIGNED_BYTE:          return 4;
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_4_4_4_4: return 2;
        default:                        return 0;
      }

    default: // unknown format
      return 0;
  }
}

static inline GLboolean tex_gl_to_nv(texture_t *tex) {
  const GLuint fmt = intfmt_gl_to_nv(tex->gl.format);

  if (fmt == 0) return GL_FALSE; // unknown format

  tex->nv.linear = GL_FALSE; // TODO: this is basically free GL_TEXTURE_RECTANGLE, but that isn't in GL until 3.1

  const GLuint addr_u = wrap_gl_to_nv(tex->gl.wrap_s);
  const GLuint wrap_u = 0; // TODO: what is this?
  const GLuint addr_v = wrap_gl_to_nv(tex->gl.wrap_t);
  const GLuint wrap_v = 0; // TODO: what is this?
  const GLuint addr_p = wrap_gl_to_nv(tex->gl.wrap_r);
  const GLuint wrap_p = 0; // TODO: what is this?
  const GLuint wrap_q = 0; // TODO: what is this?

  tex->nv.wrap =
    (addr_u << 0) | (wrap_u << 4) |
    (addr_v << 8) | (wrap_v << 12) |
    (addr_p << 16) | (wrap_p << 20) |
    (wrap_q << 24);

  const GLuint filter_min = filter_gl_to_nv_min(tex->gl.min_filter);
  const GLuint filter_mag = filter_gl_to_nv_mag(tex->gl.mag_filter);

  tex->nv.filter =
    PBGL_MASK(NV097_SET_TEXTURE_FILTER_MIN, filter_min) |
    PBGL_MASK(NV097_SET_TEXTURE_FILTER_MAG, filter_mag) |
    PBGL_MASK(NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL, NV097_SET_TEXTURE_FILTER_CONVOLUTION_KERNEL_QUINCUNX) |
    PBGL_MASK(NV097_SET_TEXTURE_FILTER_MIPMAP_LOD_BIAS, 0); // TODO

  const GLuint size_u = ulog2(tex->width);
  const GLuint size_v = ulog2(tex->height);
  const GLuint size_p = ulog2(tex->depth);

  const palette_t *pal = tex->shared_palette ? tex->shared_palette : &tex->palette;
  if (pal->data && pal->width) {
    GLuint palsize;
    switch (pal->width) {
      case 32:  palsize = NV097_SET_TEXTURE_PALETTE_LENGTH_32; break;
      case 64:  palsize = NV097_SET_TEXTURE_PALETTE_LENGTH_64; break;
      case 128: palsize = NV097_SET_TEXTURE_PALETTE_LENGTH_128; break;
      default:  palsize = NV097_SET_TEXTURE_PALETTE_LENGTH_256; break;
    }
    tex->nv.palette =
      PBGL_MASK(NV097_SET_TEXTURE_PALETTE_CONTEXT_DMA, PAL_DMA_B) |
      PBGL_MASK(NV097_SET_TEXTURE_PALETTE_LENGTH, palsize) |
      ((GLuint)pal->data & 0x03FFFFC0);
  } else {
    tex->nv.palette = 0;
  }

  tex->nv.format =
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_COLOR, fmt) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_DIMENSIONALITY, tex->dimensions) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_MIPMAP_LEVELS, tex->mipcount) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_BORDER_SOURCE, tex->gl.border) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_BASE_SIZE_U, size_u) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_BASE_SIZE_V, size_v) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_BASE_SIZE_P, size_p) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_CONTEXT_DMA, TEX_DMA_B) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_BORDER_SOURCE, NV097_SET_TEXTURE_FORMAT_BORDER_SOURCE_COLOR);

  tex->nv.addr = (GLuint)tex->data & 0x03FFFFFF;

  return GL_TRUE;
}

static inline void tex_mip_width32(GLubyte *out, GLuint out_size, const GLubyte *in, GLuint bytespp) {
  for (GLuint i = 0; i < out_size; i += bytespp, out += bytespp, in += bytespp * 2) {
    for (GLuint b = 0; b < bytespp; ++b)
      out[b] = (in[b] + in[b + bytespp]) >> 1;
  }
}

static inline void tex_mip_height32(GLubyte *out, GLuint out_pitch, GLuint out_h, const GLubyte *in, GLuint bytespp) {
  for (GLuint i = 0; i < out_h; i++, in += out_pitch) {
    for (GLuint j = 0; j < out_pitch; j += bytespp, out += bytespp, in += bytespp) {
      for (GLuint b = 0; b < bytespp; ++b)
        out[b] = (in[b] + in[out_pitch + b]) >> 1;
    }
  }
}

// NOTE: this is slow as fuck!
// low quality mipmapping is possible by just taking 1 out of 4 samples, but it looks like complete ass
static void tex_mip8(GLubyte *out, const GLubyte *in, GLuint w, GLuint h, const GLubyte *pal, const GLuint palnum) {
  h /= 2;

  for (GLuint i = 0; i < h; ++i, in += w) {
    for (GLuint j = 0; j < w; j += 2, in += 2, ++out) {
      // get the average RGBA value of 2x2 samples
      const GLubyte *c00 = pal + (in[    0] << 2);
      const GLubyte *c01 = pal + (in[    1] << 2);
      const GLubyte *c10 = pal + (in[w + 0] << 2);
      const GLubyte *c11 = pal + (in[w + 1] << 2);
      const GLubyte r = (c00[0] + c01[0] + c10[0] + c11[0]) >> 2;
      const GLubyte g = (c00[1] + c01[1] + c10[1] + c11[1]) >> 2;
      const GLubyte b = (c00[2] + c01[2] + c10[2] + c11[2]) >> 2;
      const GLubyte a = (c00[3] + c01[3] + c10[3] + c11[3]) >> 2;
      // find the closest RGBA value in the palette
      GLint mindist = INT_MAX;
      GLubyte mincol = in[0];
      const GLubyte *c = pal;
      for (GLuint n = 0; n < palnum; ++n, c += 4) {
        const GLshort dr = r - c[0];
        const GLshort dg = g - c[1];
        const GLshort db = b - c[2];
        const GLshort da = a - c[3];
        const GLint dist = dr * dr + dg * dg + db * db + da * da;
        if (dist < mindist) {
          mindist = dist;
          mincol = n;
        }
      }
      // put the palette index of that value into the result
      *out = mincol;
    }
  }
}

static void tex_build_mips(texture_t *tex, GLubyte *data, const GLboolean in_place) {
  GLubyte *buf;
  const GLubyte *in;
  if (in_place) {
    // use the input buffer for output as well
    in = data;
    buf = data;
  } else {
    // allocate a temp out buffer
    buf = malloc(tex->size >> 1);
    if (!buf) {
      pbgl_set_error(GL_OUT_OF_MEMORY);
      return;
    }
    in = data;
  }

  GLuint palnum;
  const GLubyte *palsrc;
  if (tex->shared_palette) {
    palsrc = tex->shared_palette->data;
    palnum = tex->shared_palette->width;
  } else {
    palsrc = tex->palette.data;
    palnum = tex->palette.width;
  }

  // NOTE: reading directly from contiguous memory where the palette is stored
  // seems to be very slow and sometimes seemingly causes hangs for some reason
  GLubyte pal[PAL_MAX_WIDTH  * PAL_BYTESPP] __attribute__((aligned(MEM_ALIGNMENT)));
  if (palnum) pbgl_aligned_copy(pal, palsrc, palnum * PAL_BYTESPP);

  for (GLuint level = tex->mipcount; level < tex->mipmax; ++level, ++tex->mipcount) {
    const GLuint bytespp = tex->bytespp;
    const GLuint out_pitch = tex->mips[level].pitch;
    const GLuint out_w = tex->mips[level].width;
    const GLuint out_h = tex->mips[level].height;
    if (bytespp >= 3) {
      // scronch down the width
      tex_mip_width32(buf, tex->mips[level].size << 1, in, bytespp);
      // scronch down the height
      tex_mip_height32(buf, out_pitch, out_h, buf, bytespp);
    } else if (bytespp == 1) {
      // scronch down the image
      tex_mip8(buf, in, tex->mips[level - 1].width, tex->mips[level - 1].height, pal, palnum);
    }
    // swizzle the fucker
    swizzle_rect(buf, out_w, out_h, tex->mips[level].data, out_pitch, bytespp);
    // make sure we're using the previous buffer as input
    in = buf;
  }

  if (!in_place) free(buf);

  // update nv parameters since mipcount has changed
  tex_gl_to_nv(tex);
}

static void tex_store_palette(palette_t *pal) {
  GLubyte *dst = pal->data;
  const GLubyte *src = pal->source;
  const GLuint intbytespp = pal->intbytespp;
  const GLuint extbytespp = pal->extbytespp;
  const GLboolean alpha = intbytespp == 4;
  if (extbytespp == 3 || extbytespp == 4) {
    // same as above; RGB is actually XRGB
    for (GLuint i = 0; i < pal->width; ++i) {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = alpha ? src[3] : 0xFF;
      dst += 4;
      src += extbytespp;
    }
  } else {
    pbgl_debug_log("unimplemented palette conversion from 0x%04x to GL_RGBA8", pal->extformat);
  }

  pal->source = NULL;
}

static void tex_store(texture_t *tex, const GLubyte *data, GLenum fmt, GLuint bytespp, GLuint level, GLuint x, GLuint y, GLuint w, GLuint h, GLboolean reverse, GLboolean genmips) {
  GLubyte *out = (GLubyte *)tex->mips[level].data;

  // store palette, if any
  palette_t *pal = tex->shared_palette ? tex->shared_palette : &tex->palette;
  if (pal->source)
    tex_store_palette(pal);

  if (tex->bytespp == bytespp && !reverse) {
    // no need for conversion
    if (x || y || w != tex->mips[level].width || h != tex->mips[level].height)
      swizzle_subrect(data, x, y, w, h, out, tex->mips[level].width, tex->mips[level].height, tex->bytespp);
    else
      swizzle_rect(data, w, h, out, tex->mips[level].pitch, tex->bytespp);
    // if requested, build mipmaps starting from the current level
    if (genmips)
      tex_build_mips(tex, (GLubyte *)data, GL_FALSE);
    return;
  }

  // allocate temporary buffer for conversion
  GLubyte *tmp = malloc(tex->mips[level].size);
  if (!tmp) {
    pbgl_set_error(GL_OUT_OF_MEMORY);
    return;
  }

  // convert

  const GLubyte *src = data;
  GLubyte *dst = tmp;

  if ((bytespp > 2) && (tex->gl.baseformat == GL_RGB || tex->gl.baseformat == GL_RGBA) && (fmt == GL_RGB || fmt == GL_RGBA)) {
    // 3 or 4 bytes per pixel and it's either RGBA->RGB or RGB->RGBA or RGB->(X)RGB (can't be RGBA->RGBA because that's handled above)
    // RGB is a special case: internally RGB is XRGB
    for (GLuint i = 0; i < w * h; ++i) {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = 0xFF;
      dst += tex->bytespp;
      src += bytespp;
    }
  } else {
    pbgl_debug_log("unimplemented conversion from 0x%04x to 0x%04x", fmt, tex->gl.baseformat);
  }

  // upload converted texture
  if (x || y || w != tex->mips[level].width || h != tex->mips[level].height)
    swizzle_subrect(tmp, x, y, w, h, out, tex->mips[level].width, tex->mips[level].height, tex->bytespp);
  else
    swizzle_rect(tmp, w, h, out, tex->mips[level].pitch, tex->bytespp);

  // if requested, build mipmaps starting from the current level
  // we can do it in place in the `tmp` buffer, since we own it
  if (genmips)
    tex_build_mips(tex, tmp, GL_TRUE);

  // free temp buffer
  free(tmp);
}

static inline void tex_init(texture_t *tex, GLuint dim, GLuint w, GLuint h, GLuint d, GLuint bytespp) {
  tex->dimensions = dim;
  tex->width = w;
  tex->height = h;
  tex->depth = d;
  tex->bytespp = bytespp;
  tex->pitch = tex->bytespp * tex->width;
  tex->zpitch = tex->pitch * tex->height;
  tex->mipcount = 0;
  tex->mipmax = 1; // there's always at least one mip
}

static GLboolean tex_alloc(texture_t *tex) {
  if (tex->mipmap && (tex->width > 1 || tex->height > 1)) {
    GLuint width = tex->width;
    GLuint height = tex->height;
    GLuint level = 0;
    GLubyte *ofs = NULL;
    mipmap_t *mip = tex->mips + level;
    tex->size = 0;

    while (width >= 1 || height >= 1) {
      mip->width = umax(width, 1);
      mip->height = umax(height, 1);
      mip->pitch = tex->bytespp * mip->width;
      mip->size = mip->pitch * mip->height;
      mip->data = ofs;
      ofs += mip->size;
      tex->size += mip->size;
      level++;
      mip++;
      width = width >> 1;
      height = height >> 1;
    }
    tex->mipmax = level;
  } else {
    tex->size = tex->pitch * tex->height;
    tex->mips[0].width = tex->width;
    tex->mips[0].height = tex->height;
    tex->mips[0].pitch = tex->pitch;
    tex->mips[0].size = tex->size;
    tex->mips[0].data = NULL;
    tex->mipmax = 1;
  }

  const GLuint palsize = tex->palette.width * PAL_BYTESPP;
  const GLuint asize = ALIGN(tex->size, PAL_ALIGN);
  tex->total = palsize ? asize + palsize : tex->size;
  tex->data = pbgl_mem_alloc(tex->total);
  if (!tex->data) {
    pbgl_set_error(GL_OUT_OF_MEMORY);
    return GL_FALSE;
  }

  tex->palette.data = palsize ? tex->data + asize : NULL;

  // turn miplevel offsets into pointers
  for (GLuint i = 0; i < tex->mipmax; ++i)
    tex->mips[i].data += (GLuint)tex->data;

  tex->allocated = GL_TRUE;

  return GL_TRUE;
}

static inline void tex_free(texture_t *tex) {
  if (tex->bound) glFinish(); // wait until texture is not used
  if (tex->data) pbgl_mem_free(tex->data);
  memset(tex, 0, sizeof(*tex));
}

static inline GLboolean tex_realloc(texture_t *tex) {
  // move the texture off to the heap to reduce contiguous memory fragmentation
  const GLuint palsize = tex->palette.width * PAL_BYTESPP;
  const GLuint oldsize = tex->mips[0].size;
  const GLuint oldtotal = oldsize + palsize;
  GLubyte *copy = malloc(oldtotal);
  if (!copy) return GL_FALSE;

  pbgl_aligned_copy(copy, tex->data, oldtotal);
  if (palsize)
    pbgl_aligned_copy(copy + oldsize, tex->palette.data, palsize);

  // realloc
  pbgl_mem_free(tex->data);
  tex->data = NULL;
  if (!tex_alloc(tex)) {
    free(copy);
    return GL_FALSE;
  }

  // restore the texture
  pbgl_aligned_copy(tex->data, copy, oldsize);
  if (palsize)
    pbgl_aligned_copy(tex->palette.data, copy + oldsize, palsize);

  free(copy);

  return GL_TRUE;
}

static GLboolean tex_alloc_shared_palette_mem(void) {
  pbgl.shared_palette_mem = pbgl_mem_alloc(PAL_MAX_WIDTH * PAL_BYTESPP * PAL_SHARED_COUNT);
  if (!pbgl.shared_palette_mem)
    return GL_FALSE;
  // set up palette data pointers
  palette_t *pal = pbgl.shared_palette;
  GLuint ofs = 0;
  for (GLuint i = 0; i < PAL_SHARED_COUNT; ++i, ++pal, ofs += PAL_MAX_WIDTH * PAL_BYTESPP)
    pal->data = pbgl.shared_palette_mem + ofs;
  return GL_TRUE;
}

GLboolean pbgl_tex_init(void) {
  textures = calloc(TEX_ALLOC_STEP, sizeof(texture_t));

  if (!textures) {
    pbgl_set_error(GL_OUT_OF_MEMORY);
    return GL_FALSE;
  }

  tex_cap = TEX_ALLOC_STEP;

  // allocate default texture

  textures[0].gl.basetarget = GL_TEXTURE_2D;
  textures[0].gl.baseformat = GL_RGBA;
  textures[0].gl.format = GL_RGBA8;
  textures[0].gl.mag_filter = GL_NEAREST;
  textures[0].gl.min_filter = GL_NEAREST;
  textures[0].gl.wrap_s = GL_REPEAT;
  textures[0].gl.wrap_t = GL_REPEAT;
  textures[0].gl.wrap_r = GL_REPEAT;
  textures[0].gl.border = GL_TRUE; // TODO

  tex_init(&textures[0], 2, 4, 4, 1, 4);
  if (!tex_alloc(&textures[0])) {
    free(textures);
    pbgl_set_error(GL_OUT_OF_MEMORY);
    return GL_FALSE;
  }
  tex_gl_to_nv(&textures[0]);

  tex_count = 1;

  return GL_TRUE;
}

void pbgl_tex_free(void) {
  for (GLuint i = 0; i < tex_count; ++i) {
    if (textures[i].used)
      pbgl_mem_free(textures[i].data);
  }
  if (pbgl.shared_palette_mem) {
    pbgl_mem_free(pbgl.shared_palette_mem);
    pbgl.shared_palette_mem = NULL;
    memset(pbgl.shared_palette, 0, sizeof(pbgl.shared_palette));
  }
  free(textures);
  textures = NULL;
}

GLuint pbgl_tex_get_cap(void) {
  return tex_cap;
}

GLuint pbgl_tex_get_num(void) {
  return tex_count;
}

GLuint pbgl_tex_get_used_memory(void) {
  GLuint size = 0;
  for (GLuint i = 0; i < tex_count; ++i) {
    const texture_t *tex = textures + i;
    if (tex->used && tex->allocated)
      size += tex->size;
  }
  return size;
}

/* GL FUNCTIONS BEGIN */

GL_API void glGenTextures(GLsizei num, GLuint *out) {
  GLsizei cnt = 0;

  for (GLuint tid = 1; tid < tex_cap && cnt < num; ++tid) {
    if (!textures[tid].used) {
      out[cnt++] = tid;
      textures[tid].used = GL_TRUE;
      // init with default gl state
      textures[tid].gl = textures[0].gl;
    }
  }

  // ran out of textures, gotta expand
  if (cnt < num) {
    const GLuint step = umax(TEX_ALLOC_STEP, num);

    texture_t *newtex = realloc(textures, sizeof(texture_t) * (tex_cap + step));
    if (!newtex) {
      pbgl_set_error(GL_OUT_OF_MEMORY);
      return;
    }

    textures = newtex;
    memset(&textures[tex_cap], 0, step * sizeof(texture_t));

    for (GLuint i = tex_cap; cnt < num; ++cnt, ++i) {
      out[cnt] = i;
      textures[i].used = GL_TRUE;
      // init with default gl state
      textures[i].gl = textures[0].gl;
    }

    tex_cap += step;
  }

  tex_count += num;
}

GL_API void glDeleteTextures(GLsizei num, const GLuint *ids) {
  for (GLsizei i = 0; i < num; ++i) {
    if (ids[i] < tex_cap)
      tex_free(textures + ids[i]);
  }
  tex_count -= num;
}

GL_API void glBindTexture(GLenum target, GLuint tex) {
  if (pbgl.imm.active || tex >= tex_cap) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  if (textures[tex].allocated && textures[tex].gl.basetarget && textures[tex].gl.basetarget != target) {
    // can't change targets after allocation
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  const GLboolean changed = (pbgl.tex[pbgl.active_tex_sv].tex != textures + tex);

  if (pbgl.tex[pbgl.active_tex_sv].tex)
    pbgl.tex[pbgl.active_tex_sv].tex->bound = GL_FALSE;

  if (tex == 0) {
    pbgl.tex[pbgl.active_tex_sv].tex = NULL;
    pbgl.tex[pbgl.active_tex_sv].enabled = GL_FALSE;
  } else {
    textures[tex].target = target;
    textures[tex].bound = GL_TRUE;
    pbgl.tex[pbgl.active_tex_sv].tex = &textures[tex];
    pbgl.tex[pbgl.active_tex_sv].enabled = GL_TRUE;
  }

  if (changed) {
    pbgl.tex[pbgl.active_tex_sv].dirty = GL_TRUE;
    pbgl.state_dirty = pbgl.tex_any_dirty = GL_TRUE;
    pbgl.texenv_dirty = GL_TRUE; // texture type might have changed
  }
}

GL_API void glTexImage2D(GLenum target, GLint level, GLint intfmt, GLsizei width, GLsizei height, GLint border, GLenum fmt, GLenum type, const void *data) {
  if (pbgl.imm.active) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  // TODO: cubemap targets
  if (target != GL_TEXTURE_2D) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  texture_t *tex = pbgl.tex[pbgl.active_tex_sv].tex;
  if (!tex) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  // why is < 0 even allowed
  if (level < 0 || level >= TEX_MAX_MIPS || width <= 0 || height <= 0) {
    pbgl_set_error(GL_INVALID_VALUE);
    return;
  }

  // TODO: multiple targets
  if (tex->target != target) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  GLboolean reverse = GL_FALSE; // TODO: actually implement GL_BGR* support
  const GLuint data_bytespp = extfmt_bytespp(fmt, type, &reverse);
  if (data_bytespp == 0) {
    // unsupported type/external format combo
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  // if the palette is not set but the color format is COLOR_INDEX, either assign a shared palette or bail
  const GLenum basefmt = intfmt_basefmt(intfmt);
  if (!tex->palette.source && basefmt == GL_COLOR_INDEX) {
    palette_t *pal = &pbgl.shared_palette[pbgl.active_shared_palette];
    if (!pbgl.flags.shared_pal || !pal->data) {
      // need a palette
      pbgl_set_error(GL_INVALID_OPERATION);
      return;
    }
    // set shared palette and make sure the texture palette is reset
    tex->shared_palette = pal;
    tex->palette.width = 0;
    tex->palette.source = 0;
    tex->palette.data = NULL;
  } else {
    // using no palette or a per-texture palette, reset shared
    tex->shared_palette = NULL;
  }

  pbgl.state_dirty = pbgl.tex_any_dirty = pbgl.tex[pbgl.active_tex_sv].dirty = GL_TRUE;
  pbgl.texenv_dirty = GL_TRUE; // texture type might have changed

  // lower to the previous power of two, we only support power of two textures
  // TODO: actually downscale
  width = uflp2(width);
  height = uflp2(height);

  if (tex->allocated) {
    // texture is already allocated, what the fuck does the user want?
    if (level > 0) {
      // user wants to upload mipmaps
      tex->mipcount = level + 1;
      if (!tex->mipmap) {
        // realloc the texture if it wasn't a mipmap before and we're making it a mipmap now
        // FIXME: how the fuck is a GL implementation supposed to know this in advance? do they all realloc memory?
        tex->mipmap = GL_TRUE;
        if (!tex_realloc(tex)) {
          pbgl_set_error(GL_OUT_OF_MEMORY);
          return;
        }
      } else if (level >= tex->mipmax) {
        // uploading more mips than the size can take is a no-op
        return;
      } else if (intfmt != tex->gl.format || width != tex->mips[level].width || height != tex->mips[level].height) {
        // incorrect size or format for this miplevel
        pbgl_set_error(GL_INVALID_VALUE);
        return;
      }
    } else if (tex->width != width || tex->height != height || tex->gl.format != intfmt) {
      // user wants to upload new texture, free this one
      pbgl_mem_free(tex->data);
      tex->data = NULL;
      tex->mipmap = GL_FALSE;
      tex->allocated = GL_FALSE;
    }
  }

  if (!tex->allocated) {
    // texture is not yet allocated or has just been deallocated
    // initalize texture (2 dimensions, 1 depth level)
    tex_init(tex, 2, width << level, height << level, 1, intfmt_bytespp(intfmt));
    // predict whether mipmaps are gonna be used
    if (target == GL_TEXTURE_2D) {
      tex->mipmap =
        (tex->gl.min_filter >= GL_NEAREST_MIPMAP_NEAREST) ||
        tex->gl.gen_mipmap || tex->mipmap_expected;
    }
    tex->mipcount = level + 1;
    // set GL formats
    tex->gl.basetarget = target;
    tex->gl.type = type;
    tex->gl.format = intfmt;
    tex->gl.baseformat = basefmt;
    // allocate memory
    if (!tex_alloc(tex)) {
      pbgl_set_error(GL_OUT_OF_MEMORY);
      return;
    }
    // convert formats and shit
    if (!tex_gl_to_nv(tex)) {
      // unknown format
      pbgl_mem_free(tex->data);
      tex->data = NULL;
      tex->allocated = GL_FALSE;
      pbgl_set_error(GL_INVALID_ENUM);
      return;
    }
  }

  if (data != NULL) {
    // upload data if provided
    const GLboolean generate_mips = (level == 0 && tex->gl.gen_mipmap);
    tex_store(tex, data, fmt, data_bytespp, level, 0, 0, width, height, reverse, generate_mips);
    // this will also generate mips if required
  }
}

GL_API void glTexSubImage2D(GLenum target, GLint level, GLint xoff, GLint yoff, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data) {
  if (pbgl.imm.active) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  // TODO: cubemap targets
  if (target != GL_TEXTURE_2D) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  texture_t *tex = pbgl.tex[pbgl.active_tex_sv].tex;
  // TODO: multiple targets
  if (!tex || !tex->allocated || tex->target != target) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  // why is < 0 even allowed
  if (level < 0 || level >= TEX_MAX_MIPS || width < 0 || height < 0) {
    pbgl_set_error(GL_INVALID_VALUE);
    return;
  }

  // TODO: take border into account for xoff and yoff
  if (xoff < 0 || yoff < 0 || xoff + width > tex->mips[level].width || yoff + height > tex->mips[level].height) {
    pbgl_set_error(GL_INVALID_VALUE);
    return;
  }

  // uploading more mipmaps than the texture can take is a no-op
  if (level >= tex->mipmax || level >= tex->mipcount)
    return;

  // empty subimages are a no-op
  if (width == 0 || height == 0)
    return;

  GLboolean reverse = GL_FALSE; // TODO: actually implement GL_BGR* support
  const GLuint data_bytespp = extfmt_bytespp(format, type, &reverse);
  if (data_bytespp == 0) {
    // unsupported type/external format combo
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  if (data != NULL) {
    // upload data if provided
    tex_store(tex, data, format, data_bytespp, level, xoff, yoff, width, height, reverse, GL_FALSE);
    // pbgl.state_dirty = pbgl.tex_any_dirty = pbgl.tex[pbgl.active_tex_sv].dirty = GL_TRUE;
  }
}

GL_API void glColorTableEXT(GLenum target, GLenum intfmt, GLsizei width, GLenum format, GLenum type, const void *data) {
  if (width != 32 && width != 64 && width != 128 && width != 256) {
    pbgl_set_error(GL_INVALID_VALUE);
    return;
  }

  if (intfmt != GL_RGBA && intfmt != GL_RGBA8 && intfmt != GL_RGB && intfmt != GL_RGB8 && intfmt != 3 && intfmt != 4) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  if (format != GL_RGBA && format != GL_RGB) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  if (type != GL_UNSIGNED_BYTE) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }

  GLboolean reverse = GL_FALSE;
  const GLuint extbytespp = extfmt_bytespp(format, type, &reverse);

  palette_t *pal;
  texture_t *tex = NULL;
  switch (target) {
    case GL_SHARED_TEXTURE_PALETTE_EXT:
    case GL_SHARED_TEXTURE_PALETTE0_PBGL ... GL_SHARED_TEXTURE_PALETTE7_PBGL:
      // shared palettes
      pal = pbgl.shared_palette +
        ((target == GL_SHARED_TEXTURE_PALETTE_EXT) ?
          pbgl.active_shared_palette :
          (target - GL_SHARED_TEXTURE_PALETTE0_PBGL));
      break;
    case GL_TEXTURE_2D:
      // per-texture palette
      tex = pbgl.tex[pbgl.active_tex_sv].tex;
      if (!tex) {
        pbgl_set_error(GL_INVALID_OPERATION);
        return;
      }
      pal = &tex->palette;
      break;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      return;
  }

  pal->width = width;
  pal->baseformat = intfmt_basefmt(intfmt);
  pal->intbytespp = pal->baseformat == GL_RGBA ? 4 : 3;
  pal->extformat = format;
  pal->extbytespp = extbytespp;
  pal->source = data;

  // upload shared palettes immediately
  if (!tex) {
    // if there is no shared palette memory allocated yet, allocate it
    if (!pbgl.shared_palette_mem && !tex_alloc_shared_palette_mem()) {
      pal->source = NULL;
      pal->width = 0;
      pbgl_set_error(GL_OUT_OF_MEMORY);
      return;
    }
    tex_store_palette(pal);
  }

  pbgl.tex_any_dirty = pbgl.tex[pbgl.active_tex_sv].dirty = GL_TRUE;
}

GL_API void glActiveSharedPalettePBGL(GLenum pal) {
  if (pal < GL_SHARED_TEXTURE_PALETTE0_PBGL || pal > GL_SHARED_TEXTURE_PALETTE7_PBGL) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }
  pbgl.active_shared_palette = pal - GL_SHARED_TEXTURE_PALETTE0_PBGL;
  pbgl.tex_any_dirty = pbgl.tex[pbgl.active_tex_sv].dirty = GL_TRUE;
}

GL_API void glTexParameteri(GLenum target, GLenum pname, GLint param) {
  if (pbgl.imm.active) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  texture_t *tex = pbgl.tex[pbgl.active_tex_sv].tex;
  if (!tex || tex->target != target) {
    // apparently this should cause a silent no-op
    return;
  }

  switch (pname) {
    case GL_TEXTURE_MIN_FILTER:         tex->gl.min_filter = param; break;
    case GL_TEXTURE_MAG_FILTER:         tex->gl.mag_filter = param; break;
    case GL_TEXTURE_WRAP_S:             tex->gl.wrap_s = param; break;
    case GL_TEXTURE_WRAP_T:             tex->gl.wrap_t = param; break;
    case GL_TEXTURE_WRAP_R:             tex->gl.wrap_r = param; break;
    case GL_GENERATE_MIPMAP:            tex->gl.gen_mipmap = param; break;
    case GL_TEXTURE_MAX_ANISOTROPY_EXT: break; // TODO
    case GL_EXPECT_MIPMAPS_PBGL:        tex->mipmap_expected = !!param; break;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      break;
  }

  tex_gl_to_nv(tex); // update internal parameters
  pbgl.state_dirty = pbgl.tex_any_dirty = pbgl.tex[pbgl.active_tex_sv].dirty = GL_TRUE;
}

GL_API void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
  // TODO
  glTexParameteri(target, pname, (GLint)param);
}

GL_API void glGetTexParameteriv(GLenum target, GLenum pname, GLint *out) {
  texture_t *tex = pbgl.tex[pbgl.active_tex_sv].tex;
  if (!tex || tex->target != target) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  switch (pname) {
    case GL_TEXTURE_MIN_FILTER:  *out = tex->gl.min_filter; break;
    case GL_TEXTURE_MAG_FILTER:  *out = tex->gl.mag_filter; break;
    case GL_TEXTURE_WRAP_S:      *out = tex->gl.wrap_s; break;
    case GL_TEXTURE_WRAP_T:      *out = tex->gl.wrap_t; break;
    case GL_TEXTURE_WRAP_R:      *out = tex->gl.wrap_r; break;
    case GL_GENERATE_MIPMAP:     *out = tex->gl.gen_mipmap; break;
    case GL_EXPECT_MIPMAPS_PBGL: *out = tex->mipmap_expected; break;
    default:
      pbgl_set_error(GL_INVALID_ENUM);
      break;
  }
}

GL_API void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *out) {
  // TODO
  pbgl_set_error(GL_INVALID_ENUM);
}

GL_API void glActiveTexture(GLenum tex) {
  if (tex < GL_TEXTURE0 || tex > GL_TEXTURE3) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }
  pbgl.active_tex_sv = tex - GL_TEXTURE0;

  // if GL_TEXTURE is the current matrix stack, point it to the one for this tex unit
  if (pbgl.mtx_current >= MTX_TEXTURE0)
    pbgl.mtx_current = MTX_TEXTURE0 + pbgl.active_tex_sv;
}

GL_API void glClientActiveTexture(GLenum tex) {
  if (tex < GL_TEXTURE0 || tex > GL_TEXTURE3) {
    pbgl_set_error(GL_INVALID_ENUM);
    return;
  }
  pbgl.active_tex_cl = tex - GL_TEXTURE0;
}

GL_API GLboolean glIsTexture(GLuint texture) {
  // FIXME: this can return GL_TRUE for objects that happen to have the same id as a live texture
  return (texture < tex_cap && textures[texture].used);
}

GL_API void glGenerateMipmap(GLenum target) {
  if (pbgl.imm.active) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  texture_t *tex = pbgl.tex[pbgl.active_tex_sv].tex;
  // TODO: 3D mipmaps?
  if (!tex || tex->target != target || target != GL_TEXTURE_2D) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  // can't generate mipmaps if there's nothing to generate from
  if (!tex->allocated || tex->mipcount == 0) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  // if the texture isn't ready for mipmaps, realloc it
  if (!tex->mipmap) {
    tex->mipmap = GL_TRUE;
    if (!tex_realloc(tex)) {
      pbgl_set_error(GL_OUT_OF_MEMORY);
      return;
    }
  }

  // unswizzle texture data
  const GLuint level = tex->mipcount - 1;
  GLubyte *tmp = malloc(tex->mips[level].size);
  if (!tmp) {
    pbgl_set_error(GL_OUT_OF_MEMORY);
    return;
  }

  unswizzle_rect(tex->mips[level].data, tex->mips[level].width, tex->mips[level].height, tmp, tex->mips[level].pitch, tex->bytespp);

  // build mips in place since we own the buffer
  tex_build_mips(tex, tmp, GL_TRUE);

  free(tmp);

  pbgl.state_dirty = pbgl.tex_any_dirty = pbgl.tex[pbgl.active_tex_sv].dirty = GL_TRUE;
}

GL_API void glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
  // TODO:
}

GL_API void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
  // TODO:
}

GL_API void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels) {
  if (pbgl.imm.active) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  texture_t *tex = pbgl.tex[pbgl.active_tex_sv].tex;
  if (!tex || tex->target != target || !tex->allocated || tex->mipcount <= level) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  if (format == tex->gl.baseformat && type == tex->gl.type) {
    unswizzle_rect(tex->mips[level].data, tex->mips[level].width, tex->mips[level].height, pixels, tex->mips[level].pitch, tex->bytespp);
  } else {
    // TODO
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }
}
