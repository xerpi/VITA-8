/*
 * Copyright (c) 2015 Sergi Granell (xerpi)
 */

#ifndef DRAW_H
#define DRAW_H

#include <psp2/types.h>

#define SCREEN_W 960
#define SCREEN_H 544

void font_draw_char(int x, int y, unsigned int color, char c);
void font_draw_string(int x, int y, unsigned int color, const char *string);
void font_draw_stringf(int x, int y, unsigned int color, const char *s, ...);

#endif
