#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "render.h"
#include "input.h"

#define SKIP(COND) { if (COND) { pc += 2; } } break

static uint8_t arithmetic(uint8_t n, uint8_t x, uint8_t y, int *flag)
{
	switch (n)
	{
	case 0x0:
		return y;
	case 0x1:
		return x | y;
	case 0x2:
		return x & y;
	case 0x3:
		return x ^ y;
	case 0x4:
	{
		int result = x + y;
		*flag = (result >= 256);
		return (*flag) ? result - 256 : result;
	}
	case 0x5:
	{
		int result = x - y;
		*flag = (result >= 0);
		return (*flag) ? result : result + 256;
	}
	case 0x6:
	{
		*flag = y & 1;
		return y >> 1;
	}
	case 0x7:
	{
		int result = y - x;
		*flag = (result >= 0);
		return (*flag) ? result : result + 256;
	}
	case 0xE:
	{
		*flag = (y & 0b1000000) >> 7;
		return y << 1;
	}
	default:
		fprintf(stderr, "unknown opcode 8XY%1X\n", n & 0xF);
		return 0;
	}
}

static void draw8(uint8_t x, uint8_t y, uint8_t n)
{
	regs[0xF] = 0;

	for (int j = 0; j < n && (y % fbheight) + j < fbheight; ++j)
	{
		uint16_t fbrow = (framebuffer[x / 8 + ((y % fbheight) + j) * (fbwidth / 8)] << 8) | (x + 8 < fbwidth ? framebuffer[x / 8 + 1 + ((y % fbheight) + j) * (fbwidth / 8)] : 0);
		uint16_t sprrow = ((memory[(addr + j) % 0x1000]) << 8) >> (x % 8);

		regs[0xF] = regs[0xF] || (sprrow & fbrow);

		fbrow ^= sprrow;

		framebuffer[x / 8 + ((y % fbheight) + j) * (fbwidth / 8)] = fbrow >> 8;
		if (x + 8 < fbwidth)
			framebuffer[x / 8 + 1 + ((y % fbheight) + j) * (fbwidth / 8)] = fbrow & 0xFF;
	}
}

static void draw16(uint8_t x, uint8_t y)
{
	regs[0xF] = 0;

	for (int j = 0; j < 16 && (y % fbheight) + j < fbheight; ++j)
	{
		uint32_t fbrow = (framebuffer[x / 8 + ((y % fbheight) + j) * (fbwidth / 8)] << 16)
			| ((x + 8 < fbwidth ? framebuffer[x / 8 + 1 + ((y % fbheight) + j) * (fbwidth / 8)] : 0) << 8)
			| (x + 16 < fbwidth ? framebuffer[x / 8 + 2 + ((y % fbheight) + j) * (fbwidth / 8)] : 0);

		uint32_t sprrow = (((memory[(addr + j * 2) % 0x1000]) << 16) | ((memory[(addr + j * 2 + 1) % 0x1000]) << 8)) >> (x % 8);

		regs[0xF] = regs[0xF] || (sprrow & fbrow);

		fbrow ^= sprrow;

		framebuffer[x / 8 + ((y % fbheight) + j) * (fbwidth / 8)] = fbrow >> 16;
		if (x + 8 < fbwidth)
			framebuffer[x / 8 + 1 + ((y % fbheight) + j) * (fbwidth / 8)] = (fbrow >> 8) & 0xFF;
		if (x + 16 < fbwidth)
			framebuffer[x / 8 + 1 + ((y % fbheight) + j) * (fbwidth / 8)] = (fbrow >> 16) & 0xFF;
	}
}

void vm_decode(uint8_t s, uint8_t x, uint8_t y, uint8_t n, uint8_t nn, uint16_t nnn)
{
	switch (s) // NOLINT(hicpp-multiway-paths-covered)
	{
	case 0x0:
	{
		if (y == 0xC)
		{
			// 00CN: Scroll screen down by N pixels [SCHIP]
			if (n == 0)
			{
				// Nothing to do
				break;
			}

			memmove(framebuffer + n * (fbwidth / 8), framebuffer, (fbwidth / 8) * fbheight - (n * fbwidth / 8));
			memset(framebuffer, 0, (n * fbwidth / 8));

			break;
		}
		else if (y == 0xD)
		{
			// 00DN: Set resolution to (n + 1) * 16, (n + 1) * 8 [DBCHIP]
			render_resize((n + 1) * 16, (n + 1) * 8);
			break;
		}

		switch (nn)
		{
		case 0xE0:
		{
			// 00E0: Clear screen
			memset(framebuffer, 0, (fbwidth / 8) * fbheight * sizeof(uint8_t));
		} break;
		case 0xEE:
		{
			// 00EE: Return from subroutine
			pc = vm_stack_pop();
		} break;
		case 0xFB:
		{
			// 00FB: Scroll screen right by four pixels [SCHIP]
			for (int j = 0; j < fbheight; ++j)
			{
				int prev = 0;

				for (int i = 0; i < fbwidth / 8; ++i)
				{
					int out = framebuffer[i + j * (fbwidth / 8)] & 0xF;

					framebuffer[i + j * (fbwidth / 8)] >>= 4;
					framebuffer[i + j * (fbwidth / 8)] |= (prev << 4);

					prev = out;
				}
			}
		} break;
		case 0xFC:
		{
			// 00FC: Scroll screen left by four pixels [SCHIP]
			for (int j = 0; j < fbheight; ++j)
			{
				int prev = 0;

				for (int i = fbwidth / 8 - 1; i >= 0; --i)
				{
					int out = framebuffer[i + j * (fbwidth / 8)] & 0xF0;

					framebuffer[i + j * (fbwidth / 8)] <<= 4;
					framebuffer[i + j * (fbwidth / 8)] |= (prev >> 4);

					prev = out;
				}
			}
		} break;
		case 0xFD:
		{
			// 00FD: Exit the interpreter [SCHIP]
			running = 0;
		} break;
		case 0xFE:
		{
			// 00FE: Set resolution to 64x32 (the default) [SCHIP]
			render_resize(64, 32);
		} break;
		case 0xFF:
		{
			// 00FF: Set resolution to 128x64 [SCHIP]
			render_resize(128, 64);
		} break;
		default:
			fprintf(stderr, "unknown opcode 00%02X\n", nn);
		}
	} break;
	case 0x1:
	{
		// 1NNN: Jump to address NNN
		pc = nnn;
	} break;
	case 0x2:
	{
		// 2NNN: Jump to subroutine NNN
		vm_stack_push(pc);
		pc = nnn;
	} break;
	case 0x3: SKIP(regs[x] == nn);		// 3XNN: Skip if X == NN
	case 0x4: SKIP(regs[x] != nn);		// 4XNN: Skip if X != NN
	case 0x5: SKIP(regs[x] == regs[y]);	// 5XY0: Skip if X == Y
	case 0x6:
	{
		// 6XNN: Store NN in X
		regs[x] = nn;
	} break;
	case 0x7:
	{
		// 7XNN: Add NN to X
		regs[x] += nn;
	} break;
	case 0x8:
	{
		// 8XYN: Arithmetic operation
		int flag = -1;
		regs[x] = arithmetic(n, regs[x], regs[y], &flag);
		if (flag >= 0)
			regs[0xF] = flag;
	} break;
	case 0x9: SKIP(regs[x] != regs[y]);	// 9XY0: Skip if X != Y
	case 0xA:
	{
		// ANNN: Set I to NNN
		addr = nnn;
	} break;
	case 0xB:
	{
		// BNNN: Jump to NNN + V0
		pc = (nnn + regs[0]) % 0x1000;
	} break;
	case 0xC:
	{
		// CXNN: Set X to random number AND NN
		regs[x] = rand() & nn; // NOLINT(cert-msc50-cpp)
	} break;
	case 0xD:
	{
		if (n == 0)
		{
			// DXY0: Draw a 16x16 sprite at X, Y [SCHIP]
			draw16(regs[x], regs[y]);
		}
		else
		{
			// DXYN: Draw a 8xN sprite at X, Y
			draw8(regs[x], regs[y], n);
		}
	} break;
	case 0xE:
	{
		switch (nn)
		{
		case 0x9E: SKIP(input_pressed(regs[x]));	// EX9E: Skip if key pressed
		case 0xA1: SKIP(!input_pressed(regs[x]));	// EXA1: Skip if key not pressed
		default:
			fprintf(stderr, "unknown opcode EX%02X\n", nn);
		}
	} break;
	case 0xF:
	{
		switch (nn)
		{
		case 0x07:
		{
			// Set X to delay
			regs[x] = delay;
		} break;
		case 0x0A:
		{
			// Wait for key-press
			waitreg = x;
		} break;
		case 0x15:
		{
			// Set delay to X
			delay = regs[x];
		} break;
		case 0x18:
		{
			// Set sound to X
			// NO-OP no sound yet
		} break;
		case 0x1E:
		{
			// Add X to I
			addr = (addr + regs[x]) % 0x1000;
		} break;
		case 0x29:
		{
			// Set I to font character for X
			addr = regs[x] * 5;
		} break;
		case 0x30:
		{
			// Set I to big font character for X [SCHIP]
			addr = 5 * 16 + regs[x] * 10;
		} break;
		case 0x33:
		{
			// BCD X into memory
			memory[addr]	 = regs[x] / 100;
			memory[addr + 1] = (regs[x] / 10) % 10;
			memory[addr + 2] = regs[x] % 10;
		} break;
		case 0x55:
		{
			// Store V0..VX into memory
			for (int i = 0; i <= x; ++i)
			{
				memory[(addr++) % 0x1000] = regs[i];
			}
		} break;
		case 0x65:
		{
			// Load V0..VX from memory
			for (int i = 0; i <= x; ++i)
			{
				regs[i] = memory[(addr++) % 0x1000];
			}
		} break;
		case 0x75:
		{
			// Save V0..VX to flags [SCHIP]
			for (int i = 0; i <= x; ++i)
			{
				flags[i] = regs[i];
			}
		} break;
		case 0x85:
		{
			// Load V0..VX from flags [SCHIP]
			for (int i = 0; i <= x; ++i)
			{
				regs[i] = flags[i];
			}
		} break;
		default:
			fprintf(stderr, "unknown opcode FX%02X\n", nn);
		}
	}
	}
}

#undef SKIP
