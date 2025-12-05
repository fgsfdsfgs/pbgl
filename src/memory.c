#include <stdlib.h>
#include <malloc.h>
#include <windows.h>
#include <x86intrin.h>
#include <string.h>

#include "pbgl.h"
#include "GL/gl.h"
#include "GL/glext.h"
#include "types.h"
#include "error.h"
#include "memory.h"

GLboolean pbgl_mem_init(void) {
  return GL_TRUE;
}

void pbgl_mem_shutdown(void) {

}

void *pbgl_mem_alloc(GLuint size) {
  void *p = MmAllocateContiguousMemoryEx(size, 0, PBGL_MAXRAM, MEM_ALIGNMENT, PAGE_WRITECOMBINE | PAGE_READWRITE);
  if (!p) {
    pbgl_debug_log("mem: failed to alloc %u bytes", size);
    return NULL;
  }
  return p;
}

void pbgl_mem_free(void *ptr) {
  if (ptr)
    MmFreeContiguousMemory(ptr);
}

// SDL_memcpySSE from SDL2
// requires everything to be 16-bytes aligned
static inline void memcpy_sse(GLubyte *__restrict dst, const GLubyte *__restrict src, const GLuint len) {
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

void pbgl_aligned_copy(void *dst, const void *src, const GLuint len) {
  // if addresses are 16-bytes aligned, copy it using SSE, otherwise use normal memcpy
  if (((GLuint)dst & 0xF) || ((GLuint)src & 0xF))
    memcpy(dst, src, len);
  else
    memcpy_sse((GLubyte *)dst, (const GLubyte *)src, len);
}
