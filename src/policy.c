#include "policy.h"

int fifoVictim(Frame *frame, int nframes, void *state)
{
	(void) frame;
	
	FifoState *fs = (FifoState*) state;

	int victim = fs->next_victim;
	fs->next_victim = (fs->next_victim + 1) % nframes;
	return victim;
}

int escVictim(Frame *frame, int nframes, void *state)
{


	return 0;
}

int mfuVictim(Frame *frame, int nframes, void *state)
{
	
	
	return 0;
}
