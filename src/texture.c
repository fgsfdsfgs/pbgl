#include <stdlib.h>
#include <windows.h>
#include <pbkit/pbkit.h>
#include <x86intrin.h>

#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "error.h"
#include "state.h"
#include "misc.h"
#include "swizzle.h"
#include "texture.h"

#define TEX_ALLOC_STEP 256
#define TEX_MAXRAM 0x03FFAFFF

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

static texture_t *textures = NULL;
static GLuint tex_count = 0;
static GLuint tex_cap = 0;

static inline GLuint ulog2(GLuint value) {
  static const GLuint tab32[32] = {
     0,  9,  1, 10, 13, 21,  2, 29,
    11, 14, 16, 18, 22, 25,  3, 30,
     8, 12, 20, 28, 15, 17, 24,  7,
    19, 27, 23,  6, 26,  5,  4, 31
  };
  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  value |= value >> 8;
  value |= value >> 16;
  return tab32[(GLuint)(value * 0x07C4ACDD) >> 27];
}

GLuint inline uflp2(GLuint x) {
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x - (x >> 1);
}

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

    default: // unknown format
      return 0;
  }
}

static inline GLuint extfmt_bytespp(const GLenum glformat, const GLuint type, GLboolean *reverse) {
  switch (glformat) {
    // case GL_COLOR_INDEX:
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_LUMINANCE:
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

// SDL_memcpySSE from SDL2
// requires everything to be 16-bytes aligned
static void memcpy_sse(GLubyte *__restrict dst, const GLubyte *__restrict src, const GLuint len) {
  __m128 values[4];
  for (GLuint i = len / 64; i--;) {
    _mm_prefetch((const char *)src, _MM_HINT_NTA);
    values[0] = *(__m128 *)(src + 0 );
    values[1] = *(__m128 *)(src + 16);
    values[2] = *(__m128 *)(src + 32);
    values[3] = *(__m128 *)(src + 48);
    _mm_stream_ps((float *)(dst + 0 ), values[0]);
    _mm_stream_ps((float *)(dst + 16), values[1]);
    _mm_stream_ps((float *)(dst + 32), values[2]);
    _mm_stream_ps((float *)(dst + 48), values[3]);
    src += 64;
    dst += 64;
  }
  if (len & 63)
    memcpy(dst, src, len & 63);
}

static inline void aligned_copy(void *dst, const void *src, const GLuint len) {
  // if addresses are 16-bytes aligned, copy it using SSE, otherwise use normal memcpy
  if (((GLuint)dst & 0xF) || ((GLuint)src & 0xF))
    memcpy(dst, src, len);
  else
    memcpy_sse((GLubyte *)dst, (const GLubyte *)src, len);
}

static void tex_store(texture_t *tex, const GLubyte *data, GLenum fmt, GLuint bytespp, GLuint level, GLuint x, GLuint y, GLuint w, GLuint h, GLboolean reverse) {
  GLubyte *out = (GLubyte *)tex->mips[level].data;

  if (tex->bytespp == bytespp && !reverse) {
    // no need for conversion
    if (x || y)
      swizzle_rect_offset(data, w, h, out, x, y, tex->mips[level].width, tex->mips[level].height, tex->mips[level].pitch, tex->bytespp);
    else
      swizzle_rect(data, w, h, out, tex->mips[level].pitch, tex->bytespp);
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
    // TODO: unimplemented
  }

  // upload converted texture
  if (x || y)
    swizzle_rect_offset(tmp, w, h, out, x, y, tex->mips[level].width, tex->mips[level].height, tex->mips[level].pitch, tex->bytespp);
  else
    swizzle_rect(tmp, w, h, out, tex->mips[level].pitch, tex->bytespp);

  // free temp buffer
  free(tmp);
}

static inline void tex_init(texture_t *tex, GLuint dim, GLuint w, GLuint h, GLuint d) {
  tex->dimensions = dim;
  tex->width = w;
  tex->height = h;
  tex->depth = d;
  tex->bytespp = intfmt_bytespp(tex->gl.format);
  tex->pitch = tex->bytespp * tex->width;
  tex->zpitch = tex->pitch * tex->height;
  tex->mipcount = 0;
  tex->mipmax = 1; // there's always at least one mip
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
    0x00004000; // this is either LOD bias or the convolution filter, dunno

  const GLuint size_u = ulog2(tex->width);
  const GLuint size_v = ulog2(tex->height);
  const GLuint size_p = ulog2(tex->depth);

  tex->nv.format =
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_COLOR, fmt) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_DIMENSIONALITY, tex->dimensions) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_MIPMAP_LEVELS, tex->mipcount) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_BORDER_SOURCE, tex->gl.border) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_BASE_SIZE_U, size_u) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_BASE_SIZE_V, size_v) |
    PBGL_MASK(NV097_SET_TEXTURE_FORMAT_BASE_SIZE_P, size_p) |
    0xA; /* dma context and other shit here, presumably */

  tex->nv.addr = (GLuint)tex->data & 0x03FFFFFF;

  return GL_TRUE;
}

static GLboolean tex_alloc(texture_t *tex) {
  if (tex->mipmap && (tex->width > 1 || tex->height > 1)) {
    GLuint width = tex->width;
    GLuint height = tex->height;
    GLuint level = 0;
    GLubyte *ofs = NULL;
    tex->size = 0;
    while (width > 1 || height > 1) {
      tex->mips[level].width = width;
      tex->mips[level].height = height;
      tex->mips[level].pitch = tex->bytespp * width;
      tex->mips[level].size = tex->mips[level].pitch * height;
      tex->mips[level].data = ofs;
      ofs += tex->mips[level].size;
      tex->size += tex->mips[level].size;
      level++;
      width = umax((width >> 1), 1);
      height = umax((height >> 1), 1);
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

  tex->data = MmAllocateContiguousMemoryEx(tex->size, 0, TEX_MAXRAM, TEX_ALIGN, 0x404);
  if (!tex->data) {
    pbgl_set_error(GL_OUT_OF_MEMORY);
    return GL_FALSE;
  }

  // turn miplevel offsets into pointers
  for (GLuint i = 0; i < tex->mipmax; ++i)
    tex->mips[i].data += (GLuint)tex->data;

  tex->allocated = GL_TRUE;

  return GL_TRUE;
}

static inline void tex_free(texture_t *tex) {
  if (tex->bound) glFinish(); // wait until texture is not used
  if (tex->data) MmFreeContiguousMemory(tex->data);
  memset(tex, 0, sizeof(*tex));
}

static inline GLboolean tex_realloc(texture_t *tex) {
  // move the texture off to the heap to reduce contiguous memory fragmentation
  void *copy = malloc(tex->size);
  if (!copy) return GL_FALSE;

  aligned_copy(copy, tex->data, tex->size);

  // realloc
  MmFreeContiguousMemory(tex->data);
  tex->data = NULL;
  if (!tex_alloc(tex)) {
    free(copy);
    return GL_FALSE;
  }

  // restore the texture
  aligned_copy(tex->data, copy, tex->mips[0].size);

  free(copy);

  return GL_TRUE;
}

static void tex_build_mips(texture_t *tex) {
  for (GLuint level = tex->mipcount; level < tex->mipmax; ++level, ++tex->mipcount) {
    const GLuint out_width = tex->mips[level].width;
    const GLuint out_height = tex->mips[level].height;
    const GLuint bytespp = tex->bytespp;
    const GLuint in_pitch = tex->mips[level - 1].pitch;
    const GLubyte *in = tex->mips[level - 1].data;
    GLubyte *out = tex->mips[level].data;
    // FIXME: this is not RGB555/RGB565/RGBA5551 aware
    for (GLuint y = 0; y < out_height; ++y, in += in_pitch) {
      const GLubyte *inp = in;
      for (GLuint x = 0; x < out_width; ++x, out += bytespp, inp += bytespp) {
        // take four samples from the previous level and average them
        for (GLuint i = 0; i < bytespp; ++i)
          out[i] = (inp[i] + inp[i + bytespp] + inp[i + in_pitch] + inp[i + in_pitch + bytespp]) >> 2;
      }
    }
  }
  // update nv parameters since mipcount has changed
  tex_gl_to_nv(tex);
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

  tex_init(&textures[0], 2, 4, 4, 1);
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
    if (textures[i].used && textures[i].data)
      MmFreeContiguousMemory(textures[i].data);
  }
  free(textures);
  textures = NULL;
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

  pbgl.state_dirty = pbgl.tex_any_dirty = pbgl.tex[pbgl.active_tex_sv].dirty = GL_TRUE;
  pbgl.texenv_dirty = GL_TRUE; // texture type might have changed

  // lower to the previous power of two, we only support power of two textures
  // TODO: actually downscale
  const GLuint old_width = width;
  const GLuint old_height = height;
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
      MmFreeContiguousMemory(tex->data);
      tex->data = NULL;
      tex->mipmap = GL_FALSE;
      tex->allocated = GL_FALSE;
    }
  }

  if (!tex->allocated) {
    // texture is not yet allocated or has just been deallocated
    // initalize texture (2 dimensions, 1 depth level)
    tex_init(tex, 2, width << level, height << level, 1);
    // predict whether mipmaps are gonna be used
    if (target == GL_TEXTURE_2D) {
      tex->mipmap =
        (tex->gl.min_filter >= GL_NEAREST_MIPMAP_NEAREST) ||
        tex->gl.gen_mipmap;
    }
    tex->mipcount = level + 1;
    // set GL formats
    tex->gl.basetarget = target;
    tex->gl.type = type;
    tex->gl.format = intfmt;
    tex->gl.baseformat = intfmt_basefmt(intfmt);
    // allocate memory
    if (!tex_alloc(tex)) {
      pbgl_set_error(GL_OUT_OF_MEMORY);
      return;
    }
    // convert formats and shit
    if (!tex_gl_to_nv(tex)) {
      // unknown format
      MmFreeContiguousMemory(tex->data);
      tex->data = NULL;
      pbgl_set_error(GL_INVALID_ENUM);
      return;
    }
  }

  if (data != NULL) {
    // upload data if provided
    tex_store(tex, data, fmt, data_bytespp, level, 0, 0, width, height, reverse);
    if (tex->gl.gen_mipmap && level == 0) {
      // generate mipmaps if needed
      tex_build_mips(tex);
    }
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
  if (level < 0 || level >= TEX_MAX_MIPS || width <= 0 || height <= 0) {
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

  GLboolean reverse = GL_FALSE; // TODO: actually implement GL_BGR* support
  const GLuint data_bytespp = extfmt_bytespp(format, type, &reverse);
  if (data_bytespp == 0) {
    // unsupported type/external format combo
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  if (data != NULL) {
    // upload data if provided
    tex_store(tex, data, format, data_bytespp, level, xoff, yoff, width, height, reverse);
    // pbgl.state_dirty = pbgl.tex_any_dirty = pbgl.tex[pbgl.active_tex_sv].dirty = GL_TRUE;
  }
}

GL_API void glTexParameteri(GLenum target, GLenum pname, GLint param) {
  if (pbgl.imm.active) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  texture_t *tex = pbgl.tex[pbgl.active_tex_sv].tex;
  if (!tex || tex->target != target) {
    pbgl_set_error(GL_INVALID_OPERATION);
    return;
  }

  switch (pname) {
    case GL_TEXTURE_MIN_FILTER: tex->gl.min_filter = param; break;
    case GL_TEXTURE_MAG_FILTER: tex->gl.mag_filter = param; break;
    case GL_TEXTURE_WRAP_S:     tex->gl.wrap_s = param; break;
    case GL_TEXTURE_WRAP_T:     tex->gl.wrap_t = param; break;
    case GL_TEXTURE_WRAP_R:     tex->gl.wrap_r = param; break;
    case GL_GENERATE_MIPMAP:    tex->gl.gen_mipmap = param; break;
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
    case GL_TEXTURE_MIN_FILTER: *out = tex->gl.min_filter; break;
    case GL_TEXTURE_MAG_FILTER: *out = tex->gl.mag_filter; break;
    case GL_TEXTURE_WRAP_S:     *out = tex->gl.wrap_s; break;
    case GL_TEXTURE_WRAP_T:     *out = tex->gl.wrap_t; break;
    case GL_TEXTURE_WRAP_R:     *out = tex->gl.wrap_r; break;
    case GL_GENERATE_MIPMAP:    *out = tex->gl.gen_mipmap; break;
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

  tex_build_mips(tex);

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
