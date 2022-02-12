/*
 * Copyright (c) 2015 Sergi Granell (xerpi)
 */

#ifndef DRAW_H
#define DRAW_H

#include <psp2/types.h>

#define RGBA8(r, g, b, a)      ((((a)&0xFF)<<24) | (((b)&0xFF)<<16) | (((g)&0xFF)<<8) | (((r)&0xFF)<<0))

#define SCREEN_PITCH 960
#define SCREEN_W 960
#define SCREEN_H 544

#define RED   RGBA8(255, 0,   0,   255)
#define GREEN RGBA8(0,   255, 0,   255)
#define BLUE  RGBA8(0,   0,   255, 255)
#define CYAN  RGBA8(0,   255, 255, 255)
#define LIME  RGBA8(50,  205, 50,  255)
#define PURP  RGBA8(147, 112, 219, 255)
#define WHITE RGBA8(255, 255, 255, 255)
#define BLACK RGBA8(0,   0,   0,   255)

int framebuffer_map(void);
void framebuffer_unmap(void);
int framebuffer_is_mapped(void);
void clear_screen();
void draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void draw_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void draw_circle(uint32_t x, uint32_t y, uint32_t radius, uint32_t color);

#endif
