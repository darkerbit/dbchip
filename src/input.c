#include "input.h"
#include "vm.h"

static int keys[16];

static SDL_Scancode keysyms[] = {
	SDL_SCANCODE_X,
	SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
	SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E,
	SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D,
	SDL_SCANCODE_Z, SDL_SCANCODE_C,
	SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V
};

void input_event(uint32_t type, SDL_Keysym keysym)
{
	for (int i = 0; i < 16; ++i)
	{
		if (keysym.scancode == keysyms[i])
		{
			keys[i] = (type == SDL_KEYDOWN);

			if (waitreg >= 0 && type == SDL_KEYDOWN)
			{
				regs[waitreg] = i;
				waitreg = -1;
			}
		}
	}
}

int input_pressed(uint8_t key)
{
	return keys[key];
}
