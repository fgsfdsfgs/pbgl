#ifndef _PBGL_MEMORY_H
#define _PBGL_MEMORY_H

#include "types.h"
#include "GL/gl.h"

#define MEM_ALIGNMENT 16
#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))

GLboolean pbgl_mem_init(void);
void pbgl_mem_shutdown(void);

void *pbgl_mem_alloc(GLuint size);
void pbgl_mem_free(void *ptr);

void pbgl_aligned_copy(void *dst, const void *src, const GLuint len);

#endif // _PBGL_MEMORY_H
