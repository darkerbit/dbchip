#include <stdlib.h>

#include "render.h"
#include "input.h"

uint8_t *framebuffer;
int fbwidth, fbheight;

static SDL_Texture *fbtex;

SDL_Window *window;
SDL_Renderer *renderer;

static char *error;

static int fb_vertical;

int render_init(int vertical)
{
	fb_vertical = vertical;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		error = malloc(512 * sizeof(char));
		sprintf(error, "failed to initialize SDL (%s)\n", SDL_GetError());
		return 0;
	}

	if ((window = SDL_CreateWindow(
		"darkerbit's CHIP-8 emulator",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		vertical ? 320 : 1280, 640, 0
		)) == NULL)
	{
		error = malloc(512 * sizeof(char));
		sprintf(error, "failed to create window (%s)\n", SDL_GetError());

		SDL_Quit();
		return 0;
	}

	if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == NULL)
	{
		error = malloc(512 * sizeof(char));
		sprintf(error, "failed to create renderer (%s)\n", SDL_GetError());

		SDL_DestroyWindow(window);
		SDL_Quit();
		return 0;
	}

	// Allocate framebuffer
	render_resize(64, 32);

	return 1;
}

void render_quit()
{
	free((void *) framebuffer);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

char *render_get_error()
{
	return error;
}

void render_resize(int width, int height)
{
	fbwidth = width;
	fbheight = height;

	if (framebuffer)
	{
		free((void *) framebuffer);
	}

	framebuffer = (uint8_t *) calloc((fbwidth / 8) * fbheight, sizeof(uint8_t));

	if (fbtex)
	{
		SDL_DestroyTexture(fbtex);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	fbtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, fbwidth, fbheight);
}

static void blit()
{
	uint8_t *pixels;
	int pitch;

	SDL_LockTexture(fbtex, NULL, (void **) &pixels, &pitch);

	for (int i = 0; i < fbheight * pitch; i += 4)
	{
		memset(pixels + i, (framebuffer[i / 4 / 8] & (1 << (7 - ((i / 4) % 8)))) ? 255 : 0, 3);
		pixels[i + 3] = 255;
	}

	SDL_UnlockTexture(fbtex);
}

int render_new_frame()
{
	SDL_Event e;

	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_QUIT:
			return 0;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
			input_event(e.key.type, e.key.keysym);
			break;
		}
	}

	return 1;
}

void render()
{
	blit();

	SDL_Rect dstrect = { 0, 0, fb_vertical ? 640 : 1280, fb_vertical ? 320 : 640 };
	SDL_Point center = { 640 / 2, 320 };

	SDL_RenderCopyEx(renderer, fbtex, NULL, &dstrect, fb_vertical ? -90 : 0, &center, SDL_FLIP_NONE);
	SDL_RenderPresent(renderer);
}
