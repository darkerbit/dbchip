#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "render.h"
#include "timing.h"

static char *error;

uint8_t *memory;

static uint8_t font[] = {
#include "font.inl"
};

static uint8_t bigfont[] = {
#include "bigfont.inl"
};

static char *rom_file;
static unsigned int vm_speed;

int running = 1;

int paused = 0;

uint16_t pc;

uint8_t regs[16];
uint16_t addr;

uint8_t flags[16];

uint8_t plane;

uint8_t delay;

int waitreg;

static uint16_t stack[16];
static size_t sp;

static int load_rom(char *filename)
{
	rom_file = filename;

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
	vm_speed = speed;

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

	// Initialize VM
	memset(regs, 0, sizeof(regs));
	memset(flags, 0, sizeof(regs));
	memset(stack, 0, sizeof(stack));

	addr = 0;
	pc = 0x200;
	sp = 0;
	delay = 0;
	waitreg = -1;
	plane = 1;

	return 1;
}

void vm_quit()
{
	free((void *) memory);
}

void vm_reset()
{
	vm_quit();
	vm_init(rom_file, vm_speed);

	render_resize(64, 32);
}

char *vm_get_error()
{
	return error;
}


void vm_stack_push(uint16_t n)
{
	stack[(sp++) % 16] = n;
}

uint16_t vm_stack_pop()
{
	return stack[(--sp) % 16];
}

uint8_t vm_stack_ptr()
{
	return sp;
}

uint16_t vm_stack_get(uint8_t i)
{
	return stack[i % 16];
}

static uint8_t next()
{
	return memory[(pc++) % 0x10000];
}

void vm_step()
{
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
		if (!paused && waitreg < 0)
			vm_step();
	}
}
