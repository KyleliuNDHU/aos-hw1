#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct {
	int selftest; 		// 1 = run selftest and exit.
	const char *gen;	// "random" | "locality" | "mine"
	long nrefs;
	int npages;
	int frames;		// 0 = run all 10, 20, ..., 100
	double pwrite;
	unsigned seed;
	const char *out;	// NULL = stdout
} Options;

void optionsInit(Options *opts);

#endif
