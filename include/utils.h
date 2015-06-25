#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include "chip-8.h"

#define align_mem(addr, align) (((addr) + ((align) - 1)) & ~((align) - 1))
#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)
#define abs(x) ((x) < 0 ? (-x) : (x))

#ifdef __cplusplus
extern "C" {
#endif

int ffs(int i);

int chip8_loadrom_file(struct chip8_context *ctx, const char *path);
int chip8_loadrom_memory(struct chip8_context *ctx, const void *addr, unsigned int size);
void chip8_disp_to_buf(struct chip8_context *ctx, unsigned int *buffer);
void blit_scale(unsigned int *buffer, int x, int y, int w, int h, int scale);


#ifdef __cplusplus
}
#endif

#endif
