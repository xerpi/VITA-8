#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include "chip-8.h"

#define RGBA8(r, g, b, a)      ((((a)&0xFF)<<24) | (((b)&0xFF)<<16) | (((g)&0xFF)<<8) | (((r)&0xFF)<<0))

#define SCREEN_W 960
#define SCREEN_H 544

#define RED   RGBA8(255,0,0,255)
#define GREEN RGBA8(0,255,0,255)
#define BLUE  RGBA8(0,0,255,255)
#define BLACK RGBA8(0,0,0,255)
#define WHITE RGBA8(255,255,255,255)

#ifdef __cplusplus
extern "C" {
#endif

int ffs(int i);

int chip8_loadrom_file(struct chip8_context *ctx, const char *path);
int chip8_loadrom_memory(struct chip8_context *ctx, const void *addr, unsigned int size);
void chip8_disp_to_buf(struct chip8_context *ctx, unsigned int *buffer);
void chip8_core_dump(struct chip8_context *ctx);


#ifdef __cplusplus
}
#endif

#endif
