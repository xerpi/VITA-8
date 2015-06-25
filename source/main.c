/*
 * Copyright (c) 2015 Sergi Granell (xerpi)
 */

#include <stdio.h>

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/types.h>
#include <psp2/moduleinfo.h>

#include "chip-8.h"
#include "utils.h"
#include "draw.h"

#include "PONG2_bin.h"

PSP2_MODULE_INFO(0, 0, "VITA-8");

int main()
{
	SceCtrlData pad, old_pad;
	struct chip8_context chip8;
	int i, pause = 0;

	init_video();

	chip8_init(&chip8, 64, 32);
	chip8_loadrom_memory(&chip8, PONG2_bin, PONG2_bin_size);
	unsigned int disp_buf[chip8.disp_w*chip8.disp_h];

	int scale = 12;
	int pos_x = SCREEN_W/2 - (chip8.disp_w/2)*scale;
	int pos_y = SCREEN_H/2 - (chip8.disp_h/2)*scale;

	while (1) {
		clear_screen();

		sceCtrlPeekBufferPositive(0, &pad, 1);

		font_draw_stringf(10, 10, BLACK, "VITA-8 emulator by xerpi");

		unsigned int keys_down = pad.buttons & ~old_pad.buttons;
		unsigned int keys_up = ~pad.buttons & old_pad.buttons;

		if (keys_down & PSP2_CTRL_UP) {
			chip8_key_press(&chip8, 1);
		} else if (keys_up & PSP2_CTRL_UP) {
			chip8_key_release(&chip8, 1);
		}
		if (keys_down & PSP2_CTRL_DOWN) {
			chip8_key_press(&chip8, 4);
		} else if (keys_up & PSP2_CTRL_DOWN) {
			chip8_key_release(&chip8, 4);
		}

		if (keys_down & PSP2_CTRL_TRIANGLE) {
			chip8_key_press(&chip8, 0xC);
		} else if (keys_up & PSP2_CTRL_TRIANGLE) {
			chip8_key_release(&chip8, 0xC);
		}
		if (keys_down & PSP2_CTRL_CROSS) {
			chip8_key_press(&chip8, 0xD);
		} else if (keys_up & PSP2_CTRL_CROSS) {
			chip8_key_release(&chip8, 0xD);
		}

		if (pad.buttons & PSP2_CTRL_LTRIGGER) {
			scale--;
			if (scale < 1) scale = 1;
			/* Re-center the image */
			pos_x = SCREEN_W/2 - (chip8.disp_w/2)*scale;
			pos_y = SCREEN_H/2 - (chip8.disp_h/2)*scale;
		} else if (pad.buttons & PSP2_CTRL_RTRIGGER) {
			scale++;
			/* Don't go outside of the screen! */
			if ((chip8.disp_w*scale) > SCREEN_W) scale--;
			/* Re-center the image */
			pos_x = SCREEN_W/2 - (chip8.disp_w/2)*scale;
			pos_y = SCREEN_H/2 - (chip8.disp_h/2)*scale;
		}

		if (keys_down & PSP2_CTRL_START) {
			pause = !pause;
		}

		if (!pause) {
			for (i = 0; i < 20; i++) {
				chip8_step(&chip8);
			}
		}

		chip8_disp_to_buf(&chip8, disp_buf);

		blit_scale(disp_buf,
			pos_x,
			pos_y,
			chip8.disp_w,
			chip8.disp_h,
			scale);

		if (pause) {
			font_draw_stringf(SCREEN_W/2 - 40, SCREEN_H - 50, BLACK, "PAUSE");
		}

		old_pad = pad;
		swap_buffers();
		sceDisplayWaitVblankStart();
	}

	chip8_fini(&chip8);
	end_video();
	return 0;
}
