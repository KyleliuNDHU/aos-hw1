#include "policy.h"

/* FIFO: replace the frame loaded earliest. A circular pointer over the
 * frames is equivalent to a queue because frames are filled in order. */
int fifoVictim(Frame *frame, int nframes, void *state)
{
	(void) frame;

	FifoState *fs = (FifoState*) state;

	int victim = fs->next_victim;
	fs->next_victim = (fs->next_victim + 1) % nframes;
	return victim;
}

/* Enhanced second-chance (slides pp.64-65). Classes are (ref, dirty).
 * Up to four passes starting at the clock hand:
 *   pass 1: first (0,0)            -- nothing is modified
 *   pass 2: first (0,1); every (1,x) passed on the way gets ref cleared
 *   pass 3: first (0,0)            -- must exist now if any clean page exists
 *   pass 4: first (0,1)
 * The hand stops just after the victim. The hand does NOT move on a hit. */
int escVictim(Frame *frame, int nframes, void *state)
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

	return esc_state->hand; /* unreachable: pass 4 always succeeds; satisfies the compiler */
}

/* MFU (slides pp.69): replace the page with the largest reference count;
 * ties go to the page loaded earliest (FIFO order via loaded_at). */
int mfuVictim(Frame *frame, int nframes, void *state)
{
	(void) state;	/* MFU keeps no state between faults */
	int victim = 0;

	int i;
	for(i=0; i<nframes; i++)
	{
		if(frame[i].count > frame[victim].count || (frame[i].count == frame[victim].count && frame[i].loaded_at < frame[victim].loaded_at))
			victim = i;
	}

	return victim;
}

/* ESC on hit: hardware sets the reference bit; the hand does not move. */
void escHit     (Frame *frame, int idx)
{
	frame[idx].ref = 1;
}

/* MFU on hit: one more reference to this page. */
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

/* SR-ESC on hit: set the reference bit and count the re-reference. */
void srHit(Frame *frame, int idx)
{
	frame[idx].ref = 1;
	frame[idx].count++;
}
