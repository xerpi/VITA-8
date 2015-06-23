#include "chip-8.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "utils.h"
#include "draw.h"


#define FONT_OFFSET 0
static const uint8_t chip8_font[] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70,
	0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0,
	0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0,
	0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40,
	0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0,
	0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
	0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
	0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80
};

void chip8_init(struct chip8_context *ctx, uint8_t display_w, uint8_t display_h)
{
	ctx->disp_mem = malloc((display_w * display_h)/8);
	ctx->disp_w = display_w;
	ctx->disp_h = display_h;
	chip8_reset(ctx);
}

void chip8_reset(struct chip8_context *ctx)
{
	memset(ctx->RAM,   0, sizeof(ctx->RAM));
	memcpy(ctx->RAM+FONT_OFFSET, chip8_font, sizeof(chip8_font));
	memset(ctx->stack, 0, sizeof(ctx->stack));
	memset(&ctx->regs, 0, sizeof(ctx->regs));
	ctx->regs.PC = CHIP8_ROM_LOAD_ADDR;
	ctx->regs.SP = 0;
	ctx->keyboard = 0;
	chip8_cls(ctx);
};

void chip8_fini(struct chip8_context *ctx)
{
	if (ctx->disp_mem)
		free(ctx->disp_mem);
}

void chip8_step(struct chip8_context *ctx)
{
	uint16_t instrBE = ctx->RAM[ctx->regs.PC+1]<<8 | ctx->RAM[ctx->regs.PC];
	uint16_t instr = (instrBE & 0xFF)<<8 | (instrBE>>8);
	ctx->regs.PC+=2;

	#define OP_REG_X   (instrBE & 0xF)
	#define OP_REG_Y   ((instr>>4) & 0xF)
	#define CTX_REG_V  (ctx->regs.V)
	#define CTX_REG_X  (CTX_REG_V[OP_REG_X])
	#define CTX_REG_Y  (CTX_REG_V[OP_REG_Y])
	#define CTX_REG_PC (ctx->regs.PC)
	#define CTX_REG_SP (ctx->regs.SP)
	#define CTX_STACK  (ctx->stack)
	#define CTX_RAM	   (ctx->RAM)

	switch ((instr>>12) & 0xF) {
		case 0:
			switch (instr & 0xFF) {
		/* CLS */
			case 0xE0:
				chip8_cls(ctx);
				break;
		/* RET */
			case 0xEE:
				CTX_REG_PC = CTX_STACK[CTX_REG_SP];
				CTX_REG_SP--;
				break;
			}
			break;
	/* JP */
		case 1:
			CTX_REG_PC = (instr & 0xFFF);
			break;
	/* CALL */
		case 2:
			CTX_REG_SP++;
			CTX_STACK[CTX_REG_SP] = CTX_REG_PC;
			CTX_REG_PC = (instr & 0xFFF);
			break;
	/* SE */
		case 3:
			if (CTX_REG_X == (instr & 0xFF)) {
				CTX_REG_PC+=2;
			}
			break;
	/* SNE */
		case 4:
			if (CTX_REG_X != (instr & 0xFF)) {
				CTX_REG_PC+=2;
			}
			break;
	/* SE */
		case 5:
			if (CTX_REG_X == CTX_REG_V[OP_REG_Y]) {
				CTX_REG_PC+=2;
			}
			break;
	/* LD */
		case 6:
			CTX_REG_X = instr & 0xFF;
			break;
	/* ADD */
		case 7:
			CTX_REG_X += instr & 0xFF;
			break;
		case 8:
			switch (instr & 0xF) {
		/* LD */
			case 0:
				CTX_REG_X = CTX_REG_Y;
				break;
		/* OR */
			case 1:
				CTX_REG_X |= CTX_REG_Y;
				break;
		/* AND */
			case 2:
				CTX_REG_X &= CTX_REG_Y;
				break;
		/* XOR */
			case 3:
				CTX_REG_X ^= CTX_REG_Y;
				break;
		/* ADD */
			case 4: {
				uint16_t result = CTX_REG_X + CTX_REG_Y;
				CTX_REG_X = result & 0xFF;
				CTX_REG_V[0xF] = (result > 0xFF);
				break;
			}
		/* SUB */
			case 5: {
				int8_t result = CTX_REG_X - CTX_REG_Y;
				CTX_REG_X = result;
				CTX_REG_V[0xF] = (result > 0);
				break;
			}
		/* SHR */
			case 6:
				CTX_REG_V[0xF] = (CTX_REG_Y & 0x1);
				CTX_REG_X = CTX_REG_Y>>1;
		/* SUBN */
			case 7: {
				int8_t result = CTX_REG_Y - CTX_REG_X;
				CTX_REG_X = result;
				CTX_REG_V[0xF] = (result > 0);
				break;
			}
		/* SHL */
			case 0xE:
				CTX_REG_V[0xF] = ((CTX_REG_Y>>7) & 0x1);
				CTX_REG_X = CTX_REG_Y<<1;
				break;
			}
			break;
	/* SNE */
		case 9:
			if (CTX_REG_X != CTX_REG_Y) {
				CTX_REG_PC+=2;
			}
			break;
	/* LD I */
		case 0xA:
			ctx->regs.I = instr & 0xFFF;
			break;
	/* JP V0 */
		case 0xB:
			CTX_REG_PC = CTX_REG_V[0] + (instr & 0xFFF);
			break;
	/* RND */
		case 0xC:
			CTX_REG_X = (rand()%256) & (instr&0xFF);
			break;
	/* DRW */
		case 0xD: {
			uint8_t i, n = instr & 0xF,
				x = CTX_REG_X,
				y = CTX_REG_Y;
			for (i = 0; i < n; i++) {
				uint8_t disp_idx = x/8 + (ctx->disp_w/8)*(y+i);
				uint16_t RAM_idx = ctx->regs.I+i;
				uint8_t offset = x%8;
				CTX_REG_V[0xF] |= (ctx->disp_mem[disp_idx] ^ (CTX_RAM[RAM_idx]>>offset)) & ctx->disp_mem[disp_idx];
				CTX_REG_V[0xF] |= (ctx->disp_mem[disp_idx+1] ^ (CTX_RAM[RAM_idx]<<(8-offset))) & ctx->disp_mem[disp_idx+1];

				ctx->disp_mem[disp_idx] ^= (CTX_RAM[RAM_idx]>>offset);
				ctx->disp_mem[disp_idx+1] ^= (CTX_RAM[RAM_idx]<<(8-offset));
			}
			break;
		}
		case 0xE:
			switch (instr & 0xFF) {
		/* SKP */
			case 0x9E:
				if ((ctx->keyboard>>CTX_REG_X) & 0b1) {
					CTX_REG_PC+=2;
				}
				break;
		/* SKNP */
			case 0xA1:
				if (!((ctx->keyboard>>CTX_REG_X) & 0b1)) {
					CTX_REG_PC+=2;
				}
				break;
			}
			break;
		case 0xF:
			switch (instr & 0xFF) {
		/* LD DT */
			case 0x07:
				CTX_REG_X = ctx->regs.DT;
				break;
		/* LD KEY */
			case 0x0A: {
				uint16_t new_pressed = (ctx->keyboard ^ ctx->old_keyboard) & ctx->keyboard;
				if (new_pressed) {
					CTX_REG_X = ffs(new_pressed);
				} else {
					CTX_REG_PC-=2;
				}
				break;
			}
		/* LD DT (set) */
			case 0x15:
				ctx->regs.DT = CTX_REG_X;
				break;
		/* LD ST (set) */
			case 0x18:
				ctx->regs.ST = CTX_REG_X;
				break;
		/* ADD I */
			case 0x1E:
				ctx->regs.I += CTX_REG_X;
				break;
		/* LD sprite */
			case 0x29:
				ctx->regs.I = FONT_OFFSET+CTX_REG_X*5;
				break;
		/* LD BCD */
			case 0x33: {
				uint8_t n = CTX_REG_X;
				CTX_RAM[ctx->regs.I]   = (n/100)%10;
				CTX_RAM[ctx->regs.I+1] = (n/10)%10;
				CTX_RAM[ctx->regs.I+2] = n%10;
				break;
			}
		/* LD mpoke */
			case 0x55: {
				int i;
				for (i = 0; i <= OP_REG_X; i++) {
					CTX_RAM[ctx->regs.I+i] = CTX_REG_V[i];
				}
				ctx->regs.I += OP_REG_X+1;
				break;
			}
		/* LD mpeek */
			case 0x65: {
				int i;
				for (i = 0; i <= OP_REG_X; i++) {
					CTX_REG_V[i] = CTX_RAM[ctx->regs.I+i];
				}
				break;
				ctx->regs.I += OP_REG_X+1;
			}
			break;
		}
	}
	if (ctx->regs.DT > 0) ctx->regs.DT--;
	if (ctx->regs.ST > 0) ctx->regs.ST--;
}


void chip8_cls(struct chip8_context *ctx)
{
	memset(ctx->disp_mem, 0, (ctx->disp_w * ctx->disp_h)/8);
}

void chip8_key_press(struct chip8_context *ctx, uint8_t key)
{
	if (key < 16 && !(ctx->keyboard & (1<<key))) {
		ctx->old_keyboard = ctx->keyboard;
		ctx->keyboard |= (1<<key);
	}
}

void chip8_key_release(struct chip8_context *ctx, uint8_t key)
{
	if (key < 16 && (ctx->keyboard & (1<<key))) {
		ctx->old_keyboard = ctx->keyboard;
		ctx->keyboard &= ~(1<<key);
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

