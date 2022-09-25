#include <stdint.h>
#include <stdio.h>

#define OUT(...) do { sprintf(buf, __VA_ARGS__); return; } while (0)

void disas_syntax_octo(char *buf, uint8_t s, uint8_t x, uint8_t y, uint8_t n, uint8_t nn, uint16_t nnn)
{
	switch (s) // NOLINT(hicpp-multiway-paths-covered)
	{
	case 0x0:
	{
		if (y == 0xC)		OUT("scroll-down %d", n);
		else if (y == 0xD)	OUT("scroll-up %d", n);

		switch (nn)
		{
		case 0xE0: OUT("clear");
		case 0xEE: OUT("return");
		case 0xFB: OUT("scroll-right");
		case 0xFC: OUT("scroll-left");
		case 0xFD: OUT("exit");
		case 0xFE: OUT("lores");
		case 0xFF: OUT("hires");
		default: break;
		}
	} break;
	case 0x1: OUT("jump 0x%03X", nnn);
	case 0x2: OUT(":call 0x%03X", nnn);
	case 0x3: OUT("if v%X != 0x%02X then", x, nn);
	case 0x4: OUT("if v%X == 0x%02X then", x, nn);
	case 0x5:
	{
		switch (n)
		{
		case 0x0: OUT("if v%X != v%X then", x, y);
		case 0x2: OUT("save v%X - v%X", x, y);
		case 0x3: OUT("load v%X - v%X", x, y);
		default: break;
		}
	} break;
	case 0x6: OUT("v%X := 0x%02X", x, nn);
	case 0x7: OUT("v%X += 0x%02X", x, nn);
	case 0x8:
	{
		switch (n)
		{
		case 0x0: OUT("v%X := v%X", x, y);
		case 0x1: OUT("v%X |= v%X", x, y);
		case 0x2: OUT("v%X &= v%X", x, y);
		case 0x3: OUT("v%X ^= v%X", x, y);
		case 0x4: OUT("v%X += v%X", x, y);
		case 0x5: OUT("v%X -= v%X", x, y);
		case 0x6: OUT("v%X >>= v%X", x, y);
		case 0x7: OUT("v%X =- v%X", x, y);
		case 0xE: OUT("v%X <<= v%X", x, y);
		default: break;
		}
	} break;
	case 0x9: OUT("if v%X == v%X then", x, y);
	case 0xA: OUT("i := 0x%03X", nnn);
	case 0xB: OUT("jump0 0x%03X", nnn);
	case 0xC: OUT("v%X := random 0x%02X", x, nn);
	case 0xD: OUT("sprite v%X v%X %d", x, y, n);
	case 0xE:
	{
		switch (nn)
		{
		case 0x9E: OUT("if v%X -key then", x);
		case 0xA1: OUT("if v%X key then", x);
		default: break;
		}
	} break;
	case 0xF:
	{
		switch (nn)
		{
		case 0x00: OUT("prefix word");
		case 0x01: OUT("plane %d", x);
		case 0x02: OUT("audio");
		case 0x07: OUT("v%X := delay", x);
		case 0x0A: OUT("v%X := key", x);
		case 0x15: OUT("delay := v%X", x);
		case 0x18: OUT("buzzer := v%X", x);
		case 0x1E: OUT("i += v%X", x);
		case 0x29: OUT("i := hex v%X", x);
		case 0x30: OUT("i := bighex v%X", x);
		case 0x33: OUT("bcd v%X", x);
		case 0x3A: OUT("pitch := v%X", x);
		case 0x55: OUT("save v%X", x);
		case 0x65: OUT("load v%X", x);
		case 0x75: OUT("saveflags v%X", x);
		case 0x85: OUT("loadflags v%X", x);
		default: break;
		}
	} break;
	}

	OUT("????");
}

void disas_syntax_octo_prefix(char *buf, uint16_t nnnn)
{
	OUT("i := long 0x%04X", nnnn);
}

#undef OUT
