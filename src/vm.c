#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "timing.h"

static char *error;

uint8_t *memory;

static uint8_t font[] = {
#include "font.inl"
};

static uint8_t bigfont[] = {
#include "bigfont.inl"
};

static int load_rom(char *filename)
{
	FILE *f;
	if ((f = fopen(filename, "rb")) == NULL)
	{
		error = malloc(512 * sizeof(char));
		sprintf(error, "failed to open ROM file \"%s\" (%s)\n", filename, strerror(errno));

		return 0;
	}

	fread(memory + 0x200, sizeof(uint8_t), 0x10000 - 0x200, f);

	if (ferror(f))
	{
		error = malloc(512 * sizeof(char));
		sprintf(error, "failed to read ROM file \"%s\" (%s)\n", filename, strerror(errno));

		fclose(f);
		return 0;
	}

	fclose(f);

	return 1;
}

int vm_init(char *filename, unsigned int speed)
{
	// Allocate memory
	memory = (uint8_t *) calloc(0x10000, sizeof(uint8_t));

	// Copy fonts
	memcpy(memory, font, sizeof(font));
	memcpy(memory + sizeof(font), bigfont, sizeof(bigfont));

	// Load ROM
	if (!load_rom(filename))
	{
		free((void *) memory);
		return 0;
	}

	// Set timestep
	timing_vm_init(1000000000 / speed);

	return 1;
}

void vm_quit()
{
	free((void *) memory);
}

char *vm_get_error()
{
	return error;
}

int running = 1;

uint16_t pc = 0x200;

uint8_t regs[16];
uint16_t addr = 0;

uint8_t flags[16];

uint8_t plane = 1;

uint8_t delay = 0;

int waitreg = -1;

static uint16_t stack[16];
static size_t sp = 0;

void vm_stack_push(uint16_t n)
{
	stack[(sp++) % 16] = n;
}

uint16_t vm_stack_pop()
{
	return stack[(--sp) % 16];
}

static uint8_t next()
{
	return memory[(pc++) % 0x10000];
}

static void vm_step()
{
	if (waitreg >= 0)
		return;

	uint8_t b1 = next();
	uint8_t b2 = next();

	uint8_t s = (b1 & 0xF0) >> 4;
	uint8_t x = b1 & 0xF;
	uint8_t y = (b2 & 0xF0) >> 4;
	uint8_t n = b2 & 0xF;
	uint8_t nn = b2;
	uint16_t nnn = ((b1 & 0xF) << 8) | b2;

	vm_decode(s, x, y, n, nn, nnn);
}

void vm_run()
{
	if (delay > 0)
		--delay;

	timing_vm_clock();

	while (timing_vm_step())
	{
		vm_step();
	}
}
