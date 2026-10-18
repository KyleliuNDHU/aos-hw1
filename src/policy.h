#ifndef POLICY_H
#define POLICY_H

typedef struct {
	int page;	// 0 means empty frame
	int dirty;	// Dirty bit (1 if modified, 0 otherwise)
	int ref_bit;	// Reference bit, used by ESC
	long count;	// Access count / frequency, used by MFU
} Frame;

typedef struct {
	long faults, disk_writes, interrupts;
} Stats;

/**
 * Function pointer for page replacement policies.
 * Return the index of the victim frame to be replaced.
 * Called only when all physical frames are occupied.
 */
typedef int (*VictimFn)(Frame *frames, int nframes, void *state);

#endif
