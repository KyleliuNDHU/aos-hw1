#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "refgen.h"
#include "policy.h"
#include "sim.h"
#include "options.h"
#include "parse_tool.h"

static int nextIsNull(){ fprintf(stderr, "%s needs a value\n", argv[i]); return -1; }
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
			if(i+1 >= argc)return nextIsNull();
			if(!parseInt(argv[++i], &opts->npages)) return -1;
		}
		else if(strcmp(argv[i] ,"--frames") == 0){
			if(i+1 >= argc)return nextIsNull();
                        if(!parseInt(argv[++i], &opts->frames)) return -1;
		}
		else if(strcmp(argv[i] ,"--pwrite") == 0){
			if(i+1 >= argc)return nextIsNull();
                        if(!parseDouble(argv[++i], &opts->pwrite)) return -1;
		}
		else if(strcmp(argv[i] ,"--seed") == 0){
			if(i+1 >= argc)return nextIsNull();
			int tmp;
                        if(!parseInt(argv[++i], &tmp)) return -1;
			opts->seed = (unsigned) tmp;
		}
		else if(strcmp(argv[i] ,"--out") == 0){
			if(i+1 >= argc) return nextIsNull();
                         if(!parseOut(argv[++i], &opts->out)) return -1;
		}
		else if(strcmp(argv[i] ,"--help") == 0){
			return -1;
		}
   		else{
			fprintf(stderr, "command error: can't recognize this %s", argv[i]);
			return -1;
		}
	}
	return 0;
}

static void usage(const char *prog)
{
	fprintf(stderr, "Usage: %s --selftest\n", prog);
	fprintf(stderr, "	%s --gen random|locality|mine --npages N --frames N --pwrite P(0<=P<=1) --seed S --out stdout(default)\n", prog);
}

typedef void (*GenFn) (Ref*, long, int, double, unsigned);

static GenFn pickGen(const char *name)
{
	if(strcmp(name, "random") == 0)		return genRandom;
	if(strcmp(name, "locality") == 0)	return genLocality;
	if(strcmp(name, "mine") == 0)		return genMine;
	return NULL;
}

static int runExperiment(Options *opts)
{	
	GenFn gen = pickGen(opts->gen);
	if(gen == NULL){fprintf(stderr, "unkown generator: %s\n", opts->gen); return 1;}
	
	Ref *refs = malloc(opts->nrefs * sizeof *refs);
	if(refs == NULL)
	{
		fprintf(stderr, "malloc failed\n");
		return 1;
	}
	
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
	
	int min_frames = opts->frames ? opts->frames : 10;
	int max_frames = opts->frames ? opts->frames : 100;
	
	int nfs; // number of frames
	for(nfs = min_frames; nfs <= max_frames; nfs+=10)
	{
		FifoState fifo_st = {0};
		ESCState esc_st = {0};
		
		Policy policys[] = {{"FIFO", NULL, fifoVictim, &fifo_st}, {"ESC", escHit, escVictim, &esc_st}, {"MFU", mfuHit, mfuVictim, NULL}};
		
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
static void selftest(void)
{
	
    	int seq[] = {7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1};
    	long n = sizeof seq / sizeof seq[0];
	Ref refs[20];
	for (long i = 0; i < n; i++) 
	{
		refs[i].page = seq[i] + 1; 
		refs[i].is_write = 0; 
	}
	//printf("Reference generate pattern: %s\n", opts->gen);

	// FIFO
	printf("FIFO:\n");
    	FifoState fs = {0};
   	Policy fifo = { "FIFO", NULL, fifoVictim, &fs };
    	Stats fifo_stats = simulate(3, n, refs, &fifo);
   	printf("[selftest] %-4s faults=%ld expect=%ld\n\n", fifo.name, fifo_stats.faults, 15);
	//printf("[selftest] %-4s, faults=%ld, disk_writes=%ld, interrupts=%ld\n\n",fifo.name, fifo_stats.faults, fifo_stats.disk_writes, fifo_stats.interrupts);

	// ESC
	printf("ESC:\n");
	ESCState esc_s = {0};
        Policy esc = { "ESC", escHit, escVictim, &esc_s };
        Stats esc_stats = simulate(3, n, refs, &esc);
	printf("[selftest] %-4s faults=%ld expect=%ld\n\n",esc.name, esc_stats.faults, 15);

	// MFU
	printf("MFU:\n");
	Policy mfu = { "MFU", mfuHit, mfuVictim, NULL};
	Stats mfu_stats = simulate(3, n, refs, &mfu);
	printf("[selftest] %-4s faults=%ld expect=%ld\n\n",mfu.name, mfu_stats.faults, 15);

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
