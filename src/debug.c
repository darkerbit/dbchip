#include "debug.h"
#include "vm.h"
#include "render.h"

#include <cimgui.h>

static ImVec2 zerovec = { 0, 0 };

static bool reg_overlay = 1;
static bool stack_over = 1;
static bool mem_window = 0;
static bool dis_window = 0;

static bool demo_window = 0;

static float register_overlay(float menu_height)
{
	float width;

	ImVec2 pos = { 10, 10 + menu_height };

	igSetNextWindowPos(pos, ImGuiCond_Always, zerovec);
	if (igBegin("Registers", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
	{
		igTextDisabled("Registers");

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

		if (paused)
		{
			igSpacing();
			igText("Paused!");
		}

		width = igGetWindowWidth() + 10;
	}
	igEnd();

	return width;
}

static void stack_overlay(float reg_width, float menu_height)
{
	ImVec2 pos = { reg_width + 10, 10 + menu_height };

	igSetNextWindowPos(pos, ImGuiCond_Always, zerovec);
	if (igBegin("Stack", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
	{
		igTextDisabled("Stack");

		for (int i = vm_stack_ptr() - 1; i >= 0; --i)
		{
			igText("$%03X", vm_stack_get(i));
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
		bool go_pc = igButton("Go to PC", zerovec);

		igSameLine(0, -1.0f);
		bool go_i = igButton("Go to I", zerovec);

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
						if (go_i)
							igSetScrollHereY(0.1f);

						draw_text_bg("00", 0xFF0000A7);
					}
					else if (j + i == pc)
					{
						if (go_pc)
							igSetScrollHereY(0.1f);

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

static int disas_syntax = 0;
static const char *disas_syntax_list[] = {
	"Octo"
};

void disas_syntax_octo(char *buf, uint8_t s, uint8_t x, uint8_t y, uint8_t n, uint8_t nn, uint16_t nnn);
void disas_syntax_octo_prefix(char *buf, uint16_t nnnn);

void (*disas_syntax_func[]) (char *buf, uint8_t s, uint8_t x, uint8_t y, uint8_t n, uint8_t nn, uint16_t nnn) = {
	disas_syntax_octo
};

void (*disas_syntax_prefix_func[]) (char *buf, uint16_t nnnn) = {
	disas_syntax_octo_prefix
};

static bool stepped = 0;

static void disassembler()
{
	ImVec2 size = { 400, 400 };
	igSetNextWindowSize(size, ImGuiCond_FirstUseEver);
	if (igBegin("Disassembler", &dis_window, 0))
	{
		bool go_pc = igButton("Go to PC", zerovec);

		igSameLine(0, -1.0f);
		igCombo_Str_arr("Syntax", &disas_syntax, disas_syntax_list, 1, 0);

		if (igBeginChild_Str("Disassembly", zerovec, 1, 0))
		{
			int i = pc % 2;

			while (i < 0x1000)
			{
				if (i == pc)
				{
					if (go_pc || stepped)
						igSetScrollHereY(0.1f);

					draw_text_bg("$000", 0xFF00A700);
					igText("$%03X", i);
				}
				else
				{
					igTextDisabled("$%03X", i);
				}

				igSameLine(0, -1.0f);

				char buf[512];

				uint8_t b1 = memory[i++];
				uint8_t b2 = memory[i++];

				uint8_t s = (b1 & 0xF0) >> 4;
				uint8_t x = b1 & 0xF;
				uint8_t y = (b2 & 0xF0) >> 4;
				uint8_t n = b2 & 0xF;
				uint8_t nn = b2;
				uint16_t nnn = ((b1 & 0xF) << 8) | b2;

				if (s == 0xF && nn == 0x00 && i != pc)
				{
					int nb1 = memory[i++];
					int nb2 = memory[i++];
					disas_syntax_prefix_func[disas_syntax](buf, (nb1 << 8) | nb2);
				}
				else
				{
					disas_syntax_func[disas_syntax](buf, s, x, y, n, nn, nnn);
				}

				igText(buf);
				igSameLine(0, -1.0f);
				igTextDisabled("%02X%02X", b1, b2);

				if (s == 0xF && nn == 0x00 && i != pc)
				{
					igSameLine(0, -1.0f);
					igTextDisabled("%02X%02X", memory[i - 2], memory[i - 1]);
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
			igMenuItem_BoolPtr("Stack Overlay", "F2", &stack_over, 1);
			igMenuItem_BoolPtr("Memory Viewer", "F3", &mem_window, 1);
			igMenuItem_BoolPtr("Disassembler", "F4", &dis_window, 1);

			igSeparator();

			igMenuItem_BoolPtr("ImGui Demo Window", NULL, &demo_window, 1);

			igEndMenu();
		}

		if (igBeginMenu("VM", 1))
		{
			if (!paused && igMenuItem_Bool("Pause", "F5", 0, 1))
				paused = 1;
			else if (paused && igMenuItem_Bool("Continue", "F5", 0, 1))
				paused = 0;

			if (igMenuItem_Bool("Step", "F6", 0, paused) && paused)
			{
				vm_step();
				stepped = true;
			}

			igSeparator();

			if (igMenuItem_Bool("Reset VM", "CTRL-R", 0, 1))
				vm_reset();

			igEndMenu();
		}

		igSeparator();

		igTextDisabled("F12 to toggle debug");

		igEndMainMenuBar();
	}
	igPopStyleColor(1);

	float stack_x = 0.0f;

	if (reg_overlay)
		stack_x += register_overlay(menu_height);

	if (stack_over && vm_stack_ptr() > 0)
		stack_overlay(stack_x, menu_height);

	if (mem_window)
		memory_viewer();

	if (dis_window)
		disassembler();

	if (demo_window)
		igShowDemoWindow(&demo_window);

	if (stepped)
		stepped = false;
}

#define WINKEY(var) { if (!debug_enable) { var = 1; } else { var = !var; } return 1; }

int debug_input(SDL_KeyboardEvent e)
{
	switch (e.keysym.sym)
	{
	case SDLK_F1: WINKEY(reg_overlay)
	case SDLK_F2: WINKEY(stack_over)
	case SDLK_F3: WINKEY(mem_window)
	case SDLK_F4: WINKEY(dis_window)
	case SDLK_F5:
	{
		paused = !paused;
		return paused;
	}
	case SDLK_F6:
	{
		if (paused)
		{
			vm_step();
			stepped = true;
		}
		return 0;
	}
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
