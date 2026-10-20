/**
 * 1. Initially, every frames are empty (frame.page=0)
 * 2. For each reference, determine hit or miss.
 * 3. Hit: if is_write then set 1 to dirty, otherwise do nothing.
 *         And Some Algo. need to update their state when hit occurs,
 *         so for convenience, we will alway info POLICY.
 * 4. Miss,but there are empty frames exist:
 *         fill into that frame, and do "fault++, interrupts++" to Stats;
 *         Same, notify POLICY, but by diff. event:"load a page inside."
 * 5. Miss, and no empty frame: Ask POLICY for a victim.
 *         If the dirty bit of victim is 1 will need to do aditional operations -- disk_write++, interrupts++,
 *         otherwise, just put the new page into the frame where is used to be the victim,
 *         (dirty = is_write, ref_bit=0, count=0), faults++, interrupts++,
 *         and last thing: notify POLICY about the new loaded page.
 * 6. Return three variables of Stats.
 * 7. all specific behavior of algorithm are done by the function of POLICY,
 *    simulate(sim)itself don't know which algorithm is running now.
 */

#include <stdio.h>
#include <stdlib.h>
#include "sim.h"
#include "policy.h"
#include "refgen.h"

//return the frame index of page locate at; if miss: return -1
static int findPage(const Frame *frame, int nframes, int page)
{
        int i;
        for(i=0; i<nframes; i++)
        {
                if(frame[i].page == page) return i;
        }
        return -1;
}

static int findEmpty(const Frame *frame, int nframes)
{
        int i;
        for(i=0; i<nframes; i++)
        {
                if(frame[i].page == 0) return i;
        }
        return -1;
}

Stats simulate (int nframes, long n, Ref *refs, Policy *policy)
/**    <<__ASSUMPTION__>>
 * 1. dirty bit set 1 when ref is a write access(is_write == 1)
 * 2. ref set at hit and write in from disk; clear at escVictim (if have 2nd Round scan)
 * 3. frames initial empty: all value clear to 0.
 * 4. I don't know how to write the rule of 填滿算 fault
 * 5. is_write is set by the probability of pwrite.
 */
{	
	long clock=0;
        Stats st = {0, 0, 0};
        Frame *frame = calloc(nframes, sizeof(Frame));

        if(frame == NULL)
        {
                fprintf(stderr, "calloc failed\n");
                exit(1);
        }

        long i;
        for(i=0; i < n; i++)
        {
		clock++;
		
                int idx = findPage(frame, nframes, refs[i].page);
                if(idx >= 0)    // Hit
                {
                        if(refs[i].is_write)frame[idx].dirty = 1;
                        if(policy->on_hit != NULL)
                                policy->on_hit(frame, idx, policy->state);
                }
                else // Miss
                {
                        int slot = findEmpty(frame, nframes);
                        if(slot < 0) // no empty frame, need to choose a victim
                        {
                                slot = policy->victim(frame, nframes, policy->state);
                                if(frame[slot].dirty)
                                {
                                        st.disk_writes++;
                                        st.interrupts++;
                                }
                        }

                        frame[slot].page = refs[i].page;
                        if(refs[i].is_write) frame[slot].dirty = 1;
                        else frame[slot].dirty = 0;
                        frame[slot].ref = 1;
                        frame[slot].count = 1;
			frame[slot].loaded_at=clock;
                        st.faults++;
                        st.interrupts++;

                }
        }



        free(frame);
        frame = NULL;
        return st;
}
