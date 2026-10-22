/**
 * sim.c - page replacement simulator core.
 *
 * simulate() replays one reference string against a fixed number of frames
 * and counts page faults, disk writes and interrupts. It knows nothing about
 * any particular algorithm: every policy-specific decision goes through the
 * Policy function pointers (on_hit, victim).
 *
 * Contract:
 *  1. All frames start empty (page == 0 marks an empty frame; pages are 1..N).
 *  2. For each reference decide hit or miss by a linear search of the frames
 *     (at most 100 frames, so this is cheap enough).
 *  3. Hit: a write sets the dirty bit; the policy is notified via on_hit so it
 *     can update its own bookkeeping (ESC sets ref, MFU/SR-ESC bump count).
 *  4. Miss with an empty frame: load into it. faults++, interrupts++.
 *  5. Miss with no empty frame: ask the policy for a victim. If the victim is
 *     dirty it must be written back first: disk_writes++, interrupts++.
 *     Then load the new page: dirty = is_write, ref = 1, count = 1,
 *     loaded_at = current time. faults++, interrupts++.
 *  6. Return the three counters.
 *
 * Assumptions (also stated in the report):
 *  A1. dirty bit: set when the page is loaded by a write or written on a hit
 *      (hardware sets it on every write, slides pp.35); cleared only when the
 *      page is written back, i.e. when it is evicted.
 *  A2. reference bit: set to 1 on load and on every hit (the access that
 *      caused the fault is itself a reference, slides pp.58); cleared only by
 *      the ESC/SR-ESC hand while scanning for a victim (slides pp.63). No
 *      periodic clearing by a timer.
 *  A3. MFU count: 1 on load, +1 per hit, never reset; ties broken FIFO using
 *      loaded_at (the global reference counter, cf. slides pp.53).
 *  A4. interrupts = page faults + dirty write-backs: every time the OS has to
 *      be invoked counts once (trap on fault, I/O for write-back).
 *  A5. Cold-start misses count as page faults.
 *  A6. is_write of each reference is drawn with probability pwrite.
 */

#include <stdio.h>
#include <stdlib.h>
#include "sim.h"
#include "policy.h"
#include "refgen.h"

/* Return the frame index holding `page`, or -1 if it is not resident. */
static int findPage(const Frame *frame, int nframes, int page)
{
        int i;
        for(i=0; i<nframes; i++)
        {
                if(frame[i].page == page) return i;
        }
        return -1;
}

/* Return the index of the first empty frame (page == 0), or -1 if full. */
static int findEmpty(const Frame *frame, int nframes)
{
        int i;
        for(i=0; i<nframes; i++)
        {
                if(frame[i].page == 0) return i;
        }
        return -1;
}

Stats simulate (int nframes, long n, const Ref *refs, Policy *policy)
{
	long clock = 0;  /* global time: index of the current reference */
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
                                policy->on_hit(frame, idx);
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
