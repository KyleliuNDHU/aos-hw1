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
/**
 * ESC Rule: (ref, dirty)
 * 1st Round find (0, 0), if exists then update hand to the next, 
 * and return idx of the frame.
 * 
 * 2nd Round find (0, 1), if exists do the same thing immediately, 
 * but out of that condtion will clear ref bit of the frame at which scan idx.
 * 
 * 3rd Round same as 1st Round.
 * 4th Round same as 2nd Round without the action of clear ref bit. 
 */
{
	ESCState *esc_state = (ESCState*) state;
	// Suppose the hand of ESCState point to the start frame, 
	// means the frame that is pointed by hand initially is the first frame need to check.
	
	int p = esc_state -> hand; 	// used in the loop below 
	int victim = p;			// the victim candidate

	int i;
	for(i=0; i<nframes; i++)
	{
		if(frame[p].ref == 0 && frame[p].dirty == 0)
		{
			victim = p;
			esc_state->hand = (victim+1)%nframes;
			return victim;
		}
		p = (p+1) % nframes;
	}
	
	p = esc_state -> hand;
	for(i=0; i<nframes; i++)
	{
		if(!frame[p].ref && frame[p].dirty) // (1, X) -> (0, X) 
		{
			 victim = p;
                         esc_state->hand = (victim+1)%nframes;
                         return victim;
		}
		frame[p].ref = 0;
		p = (p+1) % nframes;
	}
	
	p = esc_state -> hand;
	for(i=0; i<nframes; i++)
	{
		if(frame[p].ref == 0 && frame[p].dirty == 0)
              	{
                        victim = p;
                        esc_state->hand = (victim+1)%nframes;
                	return victim;
              	}
                p = (p+1) % nframes;
	}
        p = esc_state -> hand;
        for(i=0; i<nframes; i++)
        {
        	if(!frame[p].ref && frame[p].dirty)
                {
          		victim = p;
             		esc_state->hand = (victim+1)%nframes;
             		return victim;
            	}
                p = (p+1) % nframes;
       	}

	return esc_state->hand; // (trash code) this line never run, just for following the request of C language.
}

int mfuVictim(Frame *frame, int nframes, void *state)
{
	int victim = 0;
	
	int i;
	for(i=0; i<nframes; i++)
	{
		if(frame[i].count > frame[victim].count || (frame[i].count == frame[victim].count && frame[i].loaded_at < frame[victim].loaded_at))
			victim = i;		
	}
	
	return victim;
}

void escHit     (Frame *frame, int idx)
{
	frame[idx].ref = 1;
}

void mfuHit     (Frame *frame, int idx)
{
	frame[idx].count++;
}
