#ifndef POLICY_H
#define POLICY_H

/* One physical frame. All fields are reset by sim.c when a page is loaded. */
typedef struct {
	int  page;       // page number held here; 0 means the frame is empty
	int  dirty;      // modify bit: 1 if written since load
	int  ref;        // reference bit, used by ESC and SR-ESC
	long loaded_at;  // reference index at load time; FIFO tie-break for MFU
	long count;      // references since load (incl. the load); MFU and SR-ESC
} Frame;

typedef struct {
	long faults, disk_writes, interrupts;
} Stats;

/**
 * Function pointer for page replacement policies.
 * Return the index of the victim frame which to be replaced.
 * Called only when there is no empty frame.
 */
typedef int (*victimFn)(Frame *frame, int nframes, void *state);

/* Per-policy private state, handed back through Policy.state. */
typedef struct { int next_victim; } FifoState;   // circular pointer to the oldest frame
typedef struct { int hand; } ESCState;           // clock hand
typedef struct { int hand; } SRState;            // clock hand (Scan-Resistant ESC)
/* MFU needs no state: it rescans all frames on every fault. */

int fifoVictim	(Frame *frame, int nframes, void *state);
int escVictim	(Frame *frame, int nframes, void *state);
int mfuVictim	(Frame *frame, int nframes, void *state);
int srVictim	(Frame *frame, int nframes, void *state);

void escHit	(Frame *frame, int idx);
void mfuHit	(Frame *frame, int idx);
void srHit	(Frame *frame, int idx);

typedef struct {
	const char *name; // "FIFO", "MFU", "ESC", "SR-ESC"; printed in the CSV
	
	// called when hit, can be NULL at FIFO
	// idx is the index of the frame which the hit page at.
	void (*on_hit)(Frame *frames, int idx);
	
	victimFn victim; // called when miss occur with no empty frame.	
	void *state; // FifoState / ESCState / SRState, or NULL for MFU
}Policy;

#endif
