#ifndef POLICY_H
#define POLICY_H

typedef struct {
	int page;  // 0 means empty frame;
	int dirty; // 1 if modified; 0 otherwise
	int ref;   // used for ESC
	long count; // used for MFU
} Frame;

typedef struct {
	long faults, disk_writes, interrupts;
} Stats;

/**
 * Function pointer for page replacement policies.
 * Return the index of the victim frame which to be replaced.
 * Called only when there is no empty frame.
 */
int (*victimFn)(Frame *frame, int nframes, void *state);

typedef struct {int next_victim;} FifoState;
typedef struct {int hand;} ESCState;
typedef struct {int max_count;}MFUState;

victimFn fifoVictim	(Frame *frame, int nframes, void *state);
victimFn escVictim	(Frame *frame, int nframes, void *state);
victimFn mfuVictim	(Frame *frame, int nframes, void *state);

typedef struct {
	const char *name; // "FIFO", "MFU", "ESC",used to table
	
	// called when hit, can be NULL at FIFO
	// idx is the index of the frame which the hit page at.
	void (*on_hit)(Frame *frames, int idx, void *state)
	
	victimFn victim; // called when miss occur with no empty frame.	
	void *state; // point to FifoState / ESCState / MFUState
}Policy;

#endif
