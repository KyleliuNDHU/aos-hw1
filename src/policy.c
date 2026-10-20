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
	ESCState *esc_state = (ESCState*) state;
	// Suppose the hand of ESCState point to the start frame, 
	// means the frame that is pointed by hand initially is the first frame need to check.
	
	int p = esc_state -> hand; 	// used in the loop below 
	int victim = search_p;			// the victim candidate

	int i;
	for(i=0; i<nframes; i++)
	{
		if(frame[p].ref == 0 && frame[p].dirty == 0)
		{
			victim = p;
			break;
		}
		
		if(frame[victim].ref > frame[p].ref) // (1, X) -> (0, X) 
		{
			victim = p;
		}
		else if((frame[victim].dirty > frame[p].dirty) && (frame[victim].ref == frame[p].ref))
		// (1,1) -> (1,0)
		{
			victim = p
		}
		else continue; // victim: (ref, dirty) = (0, 1) 
		
		p = (p+1) % nframes;
	}
	

	esc_state->hand = (victim + 1) % nframes;
	return victim;
}

int mfuVictim(Frame *frame, int nframes, void *state)
{
	MFUState *mfu_state = (MFUState*) state;
	
	int victim = mfu_state -> max_count;
	
	int search = (mfu_state->max_count + 1) % nframes;
	int sec_max_num = frame[search].count;

	while(search != victim)
	{
		if(frame[search].count > sec_max_num)
		{
			sec_max_num = frame[search].count;
			mfu_state -> max_count = search;
		}
		search = (search+1)%nframes;
	}
	
	return victim;
}
void escHit     (Frame *frame, int idx, void *stats)
{
	frame[idx].ref = 1;
}
void mfuHit     (Frame *frame, int idx, void *stats)
{
	MFUState *mfu_state = (MFUState*) state;

	frame[idx].count++;
	if(frame[idx].count > frame[mfu_state->max_count])
		mfu_state->max_count = idx;
}
