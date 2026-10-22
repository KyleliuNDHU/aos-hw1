
#ifndef OPTIONS_H
#define OPTIONS_H

/* Everything the user can set on the command line (see parseArgs in main.c). */
typedef struct {
	int selftest; 		// 1 = run selftest and exit.
	const char *gen;	// "random" | "locality" | "mine"
	long nrefs;		// length of the reference string
	int npages;		// pages are numbered 1..npages
	int frames;		// 0 = run all 10, 20, ..., 100
	double pwrite;		// probability that a reference is a write
	unsigned seed;		// srand() seed; same seed => same reference string
	const char *out;	// NULL = stdout
} Options;

Options defaultOptions(void);

#endif
