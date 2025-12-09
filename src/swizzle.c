/*
 * Texture swizzling routines from mesa's nv30_transfer.c
 * (mesa/source/src/gallium/drivers/nouveau/nv30/nv30_transfer.c)
 *
 * Slightly modified for use with pbGL.
 * Was originally based on swizzle.c by Jannik Vogel and espes,
 * so some function names were preserved.
 *
 * Original license follows:
 *
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 *
 */

#include "swizzle.h"
#include "utils.h"

static inline uint32_t swizzle2d(uint32_t v, uint32_t s) {
  v = (v | (v << 8)) & 0x00ff00ffu;
  v = (v | (v << 4)) & 0x0f0f0f0fu;
  v = (v | (v << 2)) & 0x33333333u;
  v = (v | (v << 1)) & 0x55555555u;
  return v << s;
}

static inline void swizzle2d_init(uint32_t w, uint32_t h, uint32_t *out_k, uint32_t *out_km, uint32_t *out_nx) {
  const uint32_t k = ulog2(w < h ? w : h);
  *out_k = k;
  *out_km = (1 << k) - 1;
  *out_nx = w >> k;
}

static uint32_t swizzle2d_ofs(uint32_t x, uint32_t y, uint32_t k, uint32_t km, uint32_t nx) {
  const uint32_t tx = x >> k;
  const uint32_t ty = y >> k;
  uint32_t m = swizzle2d(x & km, 0);
  m |= swizzle2d(y & km, 1);
  m += ((ty * nx) + tx) << k << k;
  return m;
}

void unswizzle_rect(const uint8_t *src_buf, uint32_t w, uint32_t h, uint8_t *dst_buf, uint32_t pitch, uint32_t bytespp) {
  uint32_t k, km, nx;
  swizzle2d_init(w, h, &k, &km, &nx);

  for (uint32_t y = 0; y < h; y++) {
    for (uint32_t x = 0; x < w; x++) {
      const uint8_t *src = src_buf + swizzle2d_ofs(x, y, k, km, nx) * bytespp;
      uint8_t *dst = dst_buf + y * pitch + x * bytespp;
      switch (bytespp) {
        case 4:
          *dst++ = *src++;
          *dst++ = *src++;
          /* fallthrough */
        case 2:
          *dst++ = *src++;
          /* fallthrough */
        default:
          *dst++ = *src++;
      }
    }
  }
}

void swizzle_rect(const uint8_t *src_buf, uint32_t w, uint32_t h, uint8_t *dst_buf, uint32_t pitch, uint32_t bytespp) {
  uint32_t k, km, nx;
  swizzle2d_init(w, h, &k, &km, &nx);

  for (uint32_t y = 0; y < h; y++) {
    for (uint32_t x = 0; x < w; x++) {
      const uint8_t *src = src_buf + y * pitch + x * bytespp;
      uint8_t *dst = dst_buf + swizzle2d_ofs(x, y, k, km, nx) * bytespp;
      switch (bytespp) {
        case 4:
          *dst++ = *src++;
          *dst++ = *src++;
          /* fallthrough */
        case 2:
          *dst++ = *src++;
          /* fallthrough */
        default:
          *dst++ = *src++;
      }
    }
  }
}

void swizzle_subrect(
  const uint8_t *sub_buf, uint32_t sub_x, uint32_t sub_y, uint32_t sub_w, uint32_t sub_h,
  uint8_t *dst_buf, uint32_t dst_w, uint32_t dst_h, uint32_t dst_bytespp
) {
  uint32_t k, km, nx;
  swizzle2d_init(dst_w, dst_h, &k, &km, &nx);

  for (uint32_t y = 0; y < sub_h; y++) {
    for (uint32_t x = 0; x < sub_w; x++) {
      uint8_t *dst = dst_buf + swizzle2d_ofs(x + sub_x, y + sub_y, k, km, nx) * dst_bytespp;
      switch (dst_bytespp) {
        case 4:
          *dst++ = *sub_buf++;
          *dst++ = *sub_buf++;
          /* fallthrough */
        case 2:
          *dst++ = *sub_buf++;
          /* fallthrough */
        default:
          *dst++ = *sub_buf++;
      }
    }
  }
}
