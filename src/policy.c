#include "policy.h"

int fifoVictim(Frame *frames, int nframes, void *state)
{
	fifoState *fs = (fifoState*) state;

	int victim = fs->next_victim;
	fs->next_victim = (s->next_victim + 1) % nframes;
	return victim;
}

int escVictim(Frame *frames, int nframes, void *state)
{



}

int mfuVictim(Frame *frames, int nframes, void *state)
{



}
