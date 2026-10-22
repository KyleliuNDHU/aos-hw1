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
	(void) state;                 /* MFU keeps no state between faults */
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

/* ---- Scan-Resistant ESC (SR-ESC), our own algorithm ----
 * Idea (not in the textbook; same spirit as InnoDB midpoint insertion / 2Q):
 * a page that has been referenced only once since it was loaded (count == 1)
 * is most likely a sequential-scan page that will never be used again, so it
 * is evicted before any page that proved itself by being re-referenced.
 * Eviction order:
 *   1. count==1 & clean   2. count==1 & dirty
 *   3. ESC (0,0)          4. ESC (0,1), clearing ref AND demoting count to 1
 *   5. ESC (0,0)          6. ESC (0,1)
 * Step 4's demotion is the aging mechanism: a page that was hot long ago but
 * has not been touched during one full hand sweep loses its protection. */
static int srScan(Frame *f, int n, int hand, int wantOnce, int wantDirty, int demote)
{
	int p = hand;
	for (int i = 0; i < n; i++) {
		if (wantOnce) {
			if (f[p].count <= 1 && f[p].dirty == wantDirty) return p;
		} else {
			if (f[p].ref == 0 && f[p].dirty == wantDirty) return p;
			if (demote && f[p].ref == 1) { f[p].ref = 0; f[p].count = 1; }
		}
		p = (p + 1) % n;
	}
	return -1;
}

int srVictim(Frame *frame, int nframes, void *state)
{
	SRState *st = (SRState *)state;
	int v;
	if ((v = srScan(frame, nframes, st->hand, 1, 0, 0)) < 0)
	if ((v = srScan(frame, nframes, st->hand, 1, 1, 0)) < 0)
	if ((v = srScan(frame, nframes, st->hand, 0, 0, 0)) < 0)
	if ((v = srScan(frame, nframes, st->hand, 0, 1, 1)) < 0)
	if ((v = srScan(frame, nframes, st->hand, 0, 0, 0)) < 0)
	     v = srScan(frame, nframes, st->hand, 0, 1, 0);
	if (v < 0) v = st->hand;                      /* unreachable, keeps C happy */
	st->hand = (v + 1) % nframes;
	return v;
}

void srHit(Frame *frame, int idx)
{
	frame[idx].ref = 1;
	frame[idx].count++;
}
