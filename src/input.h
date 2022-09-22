#ifndef DBCHIP_SRC_INPUT_H
#define DBCHIP_SRC_INPUT_H

#include <stdint.h>
#include <SDL2/SDL.h>

void input_event(uint32_t type, SDL_Keysym sym);
int input_pressed(uint8_t key);

#endif //DBCHIP_SRC_INPUT_H
