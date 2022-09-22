#ifndef DBCHIP_SRC_RENDER_H
#define DBCHIP_SRC_RENDER_H

int render_init(int vertical);
void render_quit();

char *render_get_error();

#include <stdint.h>

extern uint8_t *framebuffer;
extern int fbwidth, fbheight;

void render_resize(int width, int height);

#include <SDL2/SDL.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;

int render_new_frame();

void render();

#endif //DBCHIP_SRC_RENDER_H
