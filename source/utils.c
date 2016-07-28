#include <string.h>
#include <psp2/types.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>
#include "utils.h"
#include "font.h"

int ffs(int i)
{
	int pos = 1;
	while (!(i & 0b1)) {
		i >>= 1;
		pos++;
	}
	return pos;
}

int chip8_loadrom_file(struct chip8_context *ctx, const char *path)
{
	SceUID fd;
	if (!(fd = sceIoOpen(path, SCE_O_RDONLY, 0777))) {
		return 0;
	}

	sceIoRead(fd, &(ctx->RAM[CHIP8_ROM_LOAD_ADDR]), CHIP8_ROM_MAX_SIZE);
	sceIoClose(fd);

	return 1;
}

int chip8_loadrom_memory(struct chip8_context *ctx, const void *addr, unsigned int size)
{
	memcpy(&(ctx->RAM[CHIP8_ROM_LOAD_ADDR]), addr, size);
	return size;
}

void chip8_disp_to_buf(struct chip8_context *ctx, unsigned int *buffer)
{
	int x, y;
	unsigned int color;
	for (y = 0; y < ctx->disp_h; y++) {
		for (x = 0; x < ctx->disp_w; x++) {
			color = ((ctx->disp_mem[x/8 + (ctx->disp_w/8)*y]>>(7-x%8)) & 0b1) ? GREEN : BLUE;
			buffer[x + y*ctx->disp_w] = color;
		}
	}
}

#define DEBUG(...) font_draw_stringf(10, 500-20 + (y+=20), WHITE, __VA_ARGS__);

void chip8_core_dump(struct chip8_context *ctx)
{
	int y = 0;
	DEBUG("Registers:");
	DEBUG("V0: 0x%02X  V1: 0x%02X  V2: 0x%02X  V3: 0x%02X", ctx->regs.V[0], ctx->regs.V[1], ctx->regs.V[2], ctx->regs.V[3]);
	DEBUG("V4: 0x%02X  V5: 0x%02X  V6: 0x%02X  V7: 0x%02X", ctx->regs.V[4], ctx->regs.V[5], ctx->regs.V[6], ctx->regs.V[7]);
	DEBUG("V8: 0x%02X  V9: 0x%02X  VA: 0x%02X  VB: 0x%02X", ctx->regs.V[8], ctx->regs.V[9], ctx->regs.V[0xA], ctx->regs.V[0xB]);
	DEBUG("VC: 0x%02X  VD: 0x%02X  VE: 0x%02X  VF: 0x%02X", ctx->regs.V[0xC], ctx->regs.V[0xD], ctx->regs.V[0xE], ctx->regs.V[0xF]);
	DEBUG("I: 0x%04X  PC: 0x%04X  SP: 0x%02X", ctx->regs.I, ctx->regs.PC, ctx->regs.SP);
	DEBUG("DT: 0x%02X  ST: 0x%02X", ctx->regs.DT, ctx->regs.ST);
}

