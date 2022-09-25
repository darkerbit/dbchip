#include "debug.h"
#include "vm.h"
#include "render.h"

#include <cimgui.h>

static ImVec2 zerovec = { 0, 0 };

static bool reg_overlay = 1;
static bool mem_window = 0;
static bool dis_window = 1;

static bool demo_window = 0;

static void register_overlay(float menu_height)
{
	ImVec2 pos = { 10, 10 + menu_height };

	igSetNextWindowPos(pos, ImGuiCond_Always, zerovec);
	if (igBegin("Registers", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
	{
		if (igBeginTable("Registers and Flags", 2, 0, zerovec, 0.0f))
		{
			for (int i = 0; i < 16; ++i)
			{
				igTableNextRow(0, 0);

				igTableNextColumn();
				igText("V%1X: $%02X", i, regs[i]);
				igTableNextColumn();
				igText("F%1X: $%02X", i, flags[i]);
			}

			igEndTable();
		}

		igColumns(1, NULL, 0);
		igSpacing();

		igText("PC: $%03X", pc);
		igText("I: $%04X", addr);
		igText("SP: $%1X", vm_stack_ptr());

		igSpacing();

		igText("Delay: %d", delay);
		igText("Plane: %d", plane);

		if (waitreg >= 0)
		{
			igSpacing();
			igText("Waiting for input");
			igText("-> V%1X", waitreg);
		}
	}
	igEnd();
}

static void draw_text_bg(const char *text, ImU32 col)
{
	ImVec2 pos;
	ImVec2 text_size;

	igGetCursorScreenPos(&pos);
	igCalcTextSize(&text_size, text, NULL, 0, -1.0f);

	ImVec2 start = { pos.x - 4, pos.y - 1 };
	ImVec2 end = { pos.x + text_size.x + 4, pos.y + text_size.y + 1 };

	ImDrawList_AddRectFilled(igGetWindowDrawList(), start, end, col, 0, 0);
}

static void memory_viewer()
{
	ImVec2 size = { 440, 400 };
	igSetNextWindowSize(size, ImGuiCond_FirstUseEver);
	if (igBegin("Memory Viewer", &mem_window, 0))
	{
		igText("Key");

		igSameLine(0, -1.0f);
		draw_text_bg("Program counter", 0xFF00A700);
		igText("Program counter");

		igSameLine(0, -1.0f);
		draw_text_bg("I register", 0xFF0000A7);
		igText("I register");

		if (igBeginChild_Str("Memory View", zerovec, 1, 0))
		{
			for (int j = 0; j < 0x10000; j += 16)
			{
				igText("$%04X", j);

				for (int i = 0; i < 16; ++i)
				{
					igSameLine(0, -1.0f);

					if (j + i == addr)
					{
						draw_text_bg("00", 0xFF0000A7);
					}
					else if (j + i == pc)
					{
						draw_text_bg("00 00", 0xFF00A700);
					}

					igText("%02X", memory[j + i]);
				}
			}
		}
		igEndChild();
	}
	igEnd();
}

void debug()
{
	float menu_height;

	igPushStyleColor_U32(ImGuiCol_MenuBarBg, 0);
	if (igBeginMainMenuBar())
	{
		ImVec2 menu_size;
		igGetWindowSize(&menu_size);
		menu_height = menu_size.y;

		if (igBeginMenu("Window", 1))
		{
			igMenuItem_BoolPtr("Register Overlay", "F1", &reg_overlay, 1);
			igMenuItem_BoolPtr("Memory Viewer", "F2", &mem_window, 1);
			igMenuItem_BoolPtr("Disassembler", "F3", &dis_window, 1);

			igSeparator();

			igMenuItem_BoolPtr("ImGui Demo Window", NULL, &demo_window, 1);

			igEndMenu();
		}

		if (igBeginMenu("VM", 1))
		{
			if (igMenuItem_Bool("Reset VM", "CTRL-R", 0, 1))
				vm_reset();

			igEndMenu();
		}

		igTextDisabled("F12 to toggle debug");

		igEndMainMenuBar();
	}
	igPopStyleColor(1);

	if (reg_overlay)
		register_overlay(menu_height);

	if (mem_window)
		memory_viewer();

	if (demo_window)
		igShowDemoWindow(&demo_window);
}

#define WINKEY(var) { if (!debug_enable) { var = 1; } else { var = !var; } return 1; }

int debug_input(SDL_KeyboardEvent e)
{
	switch (e.keysym.sym)
	{
	case SDLK_F1: WINKEY(reg_overlay)
	case SDLK_F2: WINKEY(mem_window)
	case SDLK_F3: WINKEY(dis_window)
	case SDLK_r:
	{
		if (e.keysym.mod & KMOD_CTRL)
		{
			vm_reset();
		}

		return 0;
	}
	default:
		return 0;
	}
}

#undef WINKEY
