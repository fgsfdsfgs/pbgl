#ifndef _PBGL_PBGL_H
#define _PBGL_PBGL_H

#ifdef __cplusplus
extern "C" {
#endif

int pbgl_init(int init_pbkit);
void pbgl_set_swap_interval(int interval);
void pbgl_swap_buffers(void);
void pbgl_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif // _PBGL_PBGL_H
