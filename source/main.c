/*
 * Copyright (c) 2015 Sergi Granell (xerpi)
 */

#include <stdio.h>
#include <sys/syslimits.h>

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/types.h>
#include <psp2/moduleinfo.h>
#include <psp2/kernel/processmgr.h>

#include <vita2d.h>

#include "chip-8.h"
#include "utils.h"
#include "font.h"
#include "file_chooser.h"

PSP2_MODULE_INFO(0, 0, "VITA-8");

int main()
{
	char rom_file[PATH_MAX];
	SceCtrlData pad, old_pad;
	struct chip8_context chip8;
	int i, pause = 0;

	vita2d_init();

	file_choose("cache0:/", rom_file);

	chip8_init(&chip8, 64, 32);
	chip8_loadrom_file(&chip8, rom_file);

	vita2d_texture *display_tex = vita2d_create_empty_texture(64, 32);
	unsigned int *display_data = vita2d_texture_get_datap(display_tex);

	int scale = 12;
	int pos_x = SCREEN_W/2 - (chip8.disp_w/2)*scale;
	int pos_y = SCREEN_H/2 - (chip8.disp_h/2)*scale;

	while (1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);
		if (pad.buttons & PSP2_CTRL_SELECT) break;

		vita2d_start_drawing();
		vita2d_clear_screen();

		font_draw_stringf(10, 10, WHITE, "VITA-8 emulator by xerpi");

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
			// 512kHz/60 = 8.53333
			for (i = 0; i < 9; i++) {
				chip8_step(&chip8);
			}
		}

		chip8_disp_to_buf(&chip8, display_data);

		vita2d_draw_texture_scale(display_tex, pos_x, pos_y, scale, scale);

		if (pause) {
			font_draw_stringf(SCREEN_W/2 - 40, SCREEN_H - 50, WHITE, "PAUSE");
			if (keys_down & PSP2_CTRL_SQUARE) {
				chip8_reset(&chip8);
				file_choose("cache0:/", rom_file);
				chip8_loadrom_file(&chip8, rom_file);
				pause = 0;
			}
		}

		old_pad = pad;
		vita2d_end_drawing();
		vita2d_swap_buffers();
	}

	chip8_fini(&chip8);
	vita2d_fini();
	vita2d_free_texture(display_tex);
	sceKernelExitProcess(0);
	return 0;
}
