#include <time.h>
#include <stdlib.h>

#include "timing.h"

typedef struct timespec ts;

static ts ts_sub(ts a, ts b)
{
	ts out = { a.tv_sec - b.tv_sec, a.tv_nsec - b.tv_nsec };
	return out;
}

static int ts_less(ts a, ts b)
{
	if (a.tv_sec != b.tv_sec)
	{
		return a.tv_sec < b.tv_sec;
	}
	else
	{
		return a.tv_nsec < b.tv_nsec;
	}
}

static ts frame60 = { 0, 1000000000 / 60 };
static ts sleep = { 0, 1000 };

static ts start60;

void timing_60_start()
{
	clock_gettime(CLOCK_MONOTONIC, &start60);
}

void timing_60_end()
{
	ts end60;
	clock_gettime(CLOCK_MONOTONIC, &end60);

	while (ts_less(ts_sub(end60, start60), frame60))
	{
		ts rem;
		nanosleep(&sleep, &rem);

		clock_gettime(CLOCK_MONOTONIC, &end60);
	}
}

static long vm_step;
static long accumulator = 0;

static ts vm_old;

void timing_vm_init(long step)
{
	vm_step = step;

	clock_gettime(CLOCK_MONOTONIC, &vm_old);
}

void timing_vm_clock()
{
	ts time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	ts diff = ts_sub(time, vm_old);
	vm_old = time;

	accumulator += diff.tv_nsec + diff.tv_sec * 1000000000;
}

int timing_vm_step()
{
	if (accumulator >= vm_step)
	{
		accumulator -= vm_step;
		return 1;
	}

	return 0;
}

void timing_seed_random()
{
	time_t t;
	srand(time(&t)); // NOLINT(cert-msc51-cpp)
}
