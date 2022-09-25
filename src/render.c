#include <stdlib.h>

#include "render.h"
#include "input.h"
#include "debug.h"

#include "cimgui.h"
#include "imgui_impl.h"

uint8_t *framebuffer0;
uint8_t *framebuffer1;
int fbwidth, fbheight;
int fbpitch;

int debug_enable = 0;

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

	// Allocate framebuffers
	render_resize(64, 32);

	// Initialize Dear ImGui
	igCreateContext(NULL);
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);

	return 1;
}

void render_quit()
{
	free((void *) framebuffer0);
	free((void *) framebuffer1);

	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	igDestroyContext(NULL);

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
	fbpitch = width / 8;

	if (framebuffer0)
	{
		free((void *) framebuffer0);
		free((void *) framebuffer1);
	}

	framebuffer0 = (uint8_t *) calloc(fbpitch * fbheight, sizeof(uint8_t));
	framebuffer1 = (uint8_t *) calloc(fbpitch * fbheight, sizeof(uint8_t));

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
		pixels[i]		= (framebuffer0[i / 4 / 8] & (1 << (7 - ((i / 4) % 8)))) ? 255 : 0;
		pixels[i + 1]	= (framebuffer1[i / 4 / 8] & (1 << (7 - ((i / 4) % 8)))) ? (pixels[i] == 255 ? 0 : 255) : pixels[i];
		pixels[i + 2]	= pixels[i];
		pixels[i + 3]	= 255;
	}

	SDL_UnlockTexture(fbtex);
}

int render_new_frame()
{
	SDL_Event e;

	while (SDL_PollEvent(&e))
	{
		ImGui_ImplSDL2_ProcessEvent(&e);

		switch (e.type)
		{
		case SDL_QUIT:
			return 0;

		case SDL_KEYDOWN:
			if (e.key.keysym.scancode == SDL_SCANCODE_F12)
			{
				debug_enable = !debug_enable;
				break;
			}

			if (debug_input(e.key))
			{
				debug_enable = 1;
				break;
			}

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

	if (fb_vertical)
	{
		SDL_Rect dstrect = { 0, 0, 640, 320 };
		SDL_Point center = { 640 / 2, 320 };

		SDL_RenderCopyEx(renderer, fbtex, NULL, &dstrect, -90, &center, SDL_FLIP_NONE);
	}
	else
	{
		SDL_RenderCopy(renderer, fbtex, NULL, NULL);
	}

	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	igNewFrame();

	if (debug_enable)
		debug();

	igRender();
	ImGui_ImplSDLRenderer_RenderDrawData((int *) igGetDrawData());

	SDL_RenderPresent(renderer);
}
