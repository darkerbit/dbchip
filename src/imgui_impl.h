#ifndef DBCHIP_SRC_IMGUI_IMPL_H
#define DBCHIP_SRC_IMGUI_IMPL_H

#include <SDL2/SDL.h>

bool ImGui_ImplSDL2_InitForSDLRenderer(SDL_Window *window, SDL_Renderer *renderer);
void ImGui_ImplSDL2_Shutdown();
void ImGui_ImplSDL2_NewFrame();
bool ImGui_ImplSDL2_ProcessEvent(const SDL_Event *event);

bool ImGui_ImplSDLRenderer_Init(SDL_Renderer *renderer);
void ImGui_ImplSDLRenderer_Shutdown();
void ImGui_ImplSDLRenderer_NewFrame();
void ImGui_ImplSDLRenderer_RenderDrawData(int *data);

#endif //DBCHIP_SRC_IMGUI_IMPL_H
