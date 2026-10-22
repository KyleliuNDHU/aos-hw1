#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "refgen.h"
#include "policy.h"
#include "sim.h"
#include "options.h"
#include "parse_tool.h"
/* main.c - command-line front end.
 * Parses options, generates one reference string, runs every policy for
 * every frame count and writes one CSV row per (policy, frames). */

static void usage(const char *prog);

/* Fill *opts from argv. Returns 0 on success, -1 on any error (a message has
 * already been printed). Options that take a value consume the next argv. */
static int parseArgs(int argc, char *argv[], Options *opts)
{
	int i;
	for(i=1; i<argc; i++)
	{
		if(strcmp(argv[i] ,"--selftest") == 0){
				opts->selftest = 1;
		}
		else if(strcmp(argv[i] ,"--gen") == 0){
			if(i+1 >= argc){ fprintf(stderr, "%s needs a value\n", argv[i]); return -1;}
			if(!parseGen(argv[++i], &opts->gen)) return -1;
		}
		else if(strcmp(argv[i] ,"--nrefs") == 0){
			if(i+1 >= argc){ fprintf(stderr, "%s needs a value\n", argv[i]); return -1; }
			if(!parseLong(argv[++i], &opts->nrefs)) return -1;
		}
		else if(strcmp(argv[i] ,"--npages") == 0){
			if(i+1 >= argc){ fprintf(stderr, "%s needs a value\n", argv[i]); return -1; }
			if(!parseInt(argv[++i], &opts->npages)) return -1;
		}
		else if(strcmp(argv[i] ,"--frames") == 0){
			if(i+1 >= argc){ fprintf(stderr, "%s needs a value\n", argv[i]); return -1; }
                        if(!parseInt(argv[++i], &opts->frames)) return -1;
		}
		else if(strcmp(argv[i] ,"--pwrite") == 0){
			if(i+1 >= argc){ fprintf(stderr, "%s needs a value\n", argv[i]); return -1; }
                        if(!parseDouble(argv[++i], &opts->pwrite)) return -1;
		}
		else if(strcmp(argv[i] ,"--seed") == 0){
			if(i+1 >= argc){ fprintf(stderr, "%s needs a value\n", argv[i]); return -1; }
			int tmp;
                        if(!parseInt(argv[++i], &tmp)) return -1;
			opts->seed = (unsigned) tmp;
		}
		else if(strcmp(argv[i] ,"--out") == 0){
			if(i+1 >= argc){ fprintf(stderr, "%s needs a value\n", argv[i]); return -1; }
                         if(!parseOut(argv[++i], &opts->out)) return -1;
		}
		else if(strcmp(argv[i] ,"--help") == 0){
			usage(argv[0]);
			exit(0);	/* help is not an error */
		}
   		else{
			fprintf(stderr, "command error: can't recognize this %s\n", argv[i]);
			return -1;
		}
	}
	return 0;
}

static void usage(const char *prog)
{
	fprintf(stderr, "Usage: %s --selftest\n", prog);
	fprintf(stderr, "	%s --gen random|locality|mine [--nrefs N] [--npages N] [--frames N]\n", prog);
	fprintf(stderr, "	   [--pwrite P (0<=P<=1)] [--seed S] [--out FILE]\n");
	fprintf(stderr, "	defaults: random, 150000 refs, 1500 pages, frames 10..100, pwrite 0.3, seed 1, stdout\n");
}

typedef void (*GenFn) (Ref*, long, int, double, unsigned);

/* Map a generator name to its function; NULL if unknown. */
static GenFn pickGen(const char *name)
{
	if(strcmp(name, "random") == 0)		return genRandom;
	if(strcmp(name, "locality") == 0)	return genLocality;
	if(strcmp(name, "mine") == 0)		return genMine;
	return NULL;
}

/* Generate one reference string (shared by every run so the comparison is
 * paired), then for frames = 10..100 run FIFO, ESC, MFU and SR-ESC and write
 * a CSV row each. Policy state is re-created per frame count so no clock hand
 * leaks from one run into the next. Returns 0 on success. */
static int runExperiment(Options *opts)
{
	GenFn gen = pickGen(opts->gen);
	if(gen == NULL){fprintf(stderr, "unknown generator: %s\n", opts->gen); return 1;}

	Ref *refs = malloc(opts->nrefs * sizeof *refs);
	if(refs == NULL)
	{
		fprintf(stderr, "malloc failed\n");
		return 1;
	}

	gen(refs, opts->nrefs, opts->npages, opts->pwrite, opts->seed);

	FILE *out = stdout;
	if(opts->out != NULL)
		out = fopen(opts->out, "w");

	if(out == NULL)
	{
		perror(opts->out);
		free(refs); 
		refs = NULL;
		return 1;
	}

	fprintf(out, "gen,algo,frames,faults,interrupts,disk_writes\n");

	/* --frames N runs only N; otherwise sweep 10, 20, ..., 100 as the assignment asks */
	int min_frames = opts->frames ? opts->frames : 10;
	int max_frames = opts->frames ? opts->frames : 100;

	int nfs; // number of frames
	for(nfs = min_frames; nfs <= max_frames; nfs+=10)
	{
		FifoState fifo_st = {0};
		ESCState esc_st = {0};
		SRState sr_st = {0};

		/* {name, on_hit, victim, state}; FIFO has no on_hit, MFU has no state */
		Policy policys[] = {{"FIFO", NULL, fifoVictim, &fifo_st}, {"ESC", escHit, escVictim, &esc_st}, {"MFU", mfuHit, mfuVictim, NULL}, {"SR-ESC", srHit, srVictim, &sr_st}};

		int npolicys = sizeof policys / sizeof policys[0];
		for(int k=0; k<npolicys; k++)
		{
			Stats ss = simulate (nfs, opts->nrefs, refs, &policys[k]);
			fprintf(out, "%s,%s,%d,%ld,%ld,%ld\n",opts->gen, policys[k].name, nfs, ss.faults, ss.interrupts, ss.disk_writes);
		}
	}

	if(out != stdout) fclose(out);
	free(refs);
	refs = NULL;
	return 0;
}

/* Known-answer test: textbook reference string (Silberschatz ch.10) with 3
 * frames, all reads. Under assumptions A1-A3 the expected page faults are
 * FIFO 15, ESC 14, MFU 15. */
static void selftest(void)
{

    	int seq[] = {7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1};
    	long n = sizeof seq / sizeof seq[0];
	Ref refs[20];
	for (long i = 0; i < n; i++) 
	{
		refs[i].page = seq[i] + 1;	/* textbook pages are 0..7; ours must be >= 1 (0 = empty frame) */
		refs[i].is_write = 0; 
	}

	// FIFO
	printf("FIFO:\n");
    	FifoState fs = {0};
   	Policy fifo = { "FIFO", NULL, fifoVictim, &fs };
    	Stats fifo_stats = simulate(3, n, refs, &fifo);
   	printf("[selftest] %-4s faults=%ld expect=%d\n\n", fifo.name, fifo_stats.faults, 15);

	// ESC
	printf("ESC:\n");
	ESCState esc_s = {0};
        Policy esc = { "ESC", escHit, escVictim, &esc_s };
        Stats esc_stats = simulate(3, n, refs, &esc);
	printf("[selftest] %-4s faults=%ld expect=%d\n\n",esc.name, esc_stats.faults, 14);

	// MFU
	printf("MFU:\n");
	Policy mfu = { "MFU", mfuHit, mfuVictim, NULL};
	Stats mfu_stats = simulate(3, n, refs, &mfu);
	printf("[selftest] %-4s faults=%ld expect=%d\n\n",mfu.name, mfu_stats.faults, 15);

}


int main(int argc, char *argv[])
{
	Options opts = defaultOptions();

	if(parseArgs(argc, argv, &opts) < 0)
	{
		usage(argv[0]);
		return 1;
	}

	if(opts.selftest){selftest(); return 0;}
	else {return runExperiment(&opts);}

}
