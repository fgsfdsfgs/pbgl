#ifndef _PBGL_SWIZZLE_H
#define _PBGL_SWIZZLE_H

#include <stdint.h>

void unswizzle_rect(const uint8_t *src_buf, uint32_t w, uint32_t h, uint8_t *dst_buf, uint32_t pitch, uint32_t bytespp);
void swizzle_rect(const uint8_t *src_buf, uint32_t w, uint32_t h, uint8_t *dst_buf, uint32_t pitch, uint32_t bytespp);
void swizzle_subrect(
  const uint8_t *sub_buf, uint32_t sub_x, uint32_t sub_y, uint32_t sub_w, uint32_t sub_h,
  uint8_t *dst_buf, uint32_t dst_w, uint32_t dst_h, uint32_t dst_bytespp
);

#endif // _PBGL_SWIZZLE_H
