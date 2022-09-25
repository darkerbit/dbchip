#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "render.h"
#include "timing.h"

const char *usage = "darkerbit's CHIP-8 emulator\n" \
					"\n" \
					"Usage: %s [-vertical] [-uncapped] [-debug] [-speed <cycles per second>] <rom file>\n" \

					"- -vertical:\tRotates the screen 90 degrees counter-clockwise.\n" \
					"- -uncapped:\tUncaps draw from 60hz.\n" \
					"- -debug:\tTurns on debug mode.\n" \
					"- -speed:\tSets the speed of the emulator, in cycles per second.\n";

int main(int argc, char **argv)
{
	// Do we have enough arguments?
	if (argc <= 1)
	{
		printf(usage, argv[0]);
		return 1;
	}

	// Parse flags
	int vertical = 0;
	int uncapped = 0;
	unsigned int speed = 4096;

	int i;
	for (i = 1; i < argc && argv[i][0] == '-'; ++i)
	{
		if (strcmp(argv[i], "-vertical") == 0)
		{
			vertical = 1;
		}
		else if (strcmp(argv[i], "-uncapped") == 0)
		{
			uncapped = 1;
		}
		else if (strcmp(argv[i], "-debug") == 0)
		{
			debug_enable = 1;
		}
		else if (strcmp(argv[i], "-speed") == 0)
		{
			++i;

			if (i >= argc)
			{
				fprintf(stderr, "%s: -speed requires an argument\n", argv[0]);
				return 1;
			}

			if (sscanf(argv[i], "%u", &speed) != 1) // NOLINT(cert-err34-c)
			{
				fprintf(stderr, "%s: -speed expects valid unsigned integer (got \"%s\")\n", argv[0], argv[i]);
				return 1;
			}

			if (speed == 0)
			{
				fprintf(stderr, "%s: -speed must be above 0\n", argv[0]);
			}
		}
		else
		{
			fprintf(stderr, "%s: unrecognized flag \"%s\"\n", argv[0], argv[i]);
		}
	}

	// Did the user pass in too few or too many ROM files?
	if (i >= argc)
	{
		fprintf(stderr, "%s: a ROM file is required\n", argv[0]);
		return 1;
	}
	else if (i < argc - 1)
	{
		fprintf(stderr, "%s: only one ROM file can be loaded\n", argv[0]);
		return 1;
	}

	// Initialize virtual machine
	if (!vm_init(argv[i], speed))
	{
		fprintf(stderr, "%s: failed to initialize VM: %s\n", argv[0], vm_get_error());
		return 1;
	}

	// Initialize renderer
	if (!render_init(vertical))
	{
		fprintf(stderr, "%s: failed to initialize renderer: %s\n", argv[0], render_get_error());
		vm_quit();
		return 1;
	}

	timing_seed_random();

	// Main loop
	while (running)
	{
		if (!uncapped)
			timing_60_start();

		if (!render_new_frame())
			break;

		vm_run();
		render();

		if (!uncapped)
			timing_60_end();
	}

	// Exit
	render_quit();
	vm_quit();
	return 0;
}
