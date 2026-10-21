#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "refgen.h"
#include "policy.h"
#include "sim.h"
#include "options.h"
#include "parse_tool.h"

static int parseArgs(int argc, char *argv[], Options *opts)
{
	int i;
	for(i=0; i<argc; i++)
	{	
		if(strcmp(argv[i] ,"--selftest":) == 0)
				opts->selftest = 1;

		else if(strcmp(argv[i] ,"--gen") == 0)
			if(!parseGen(argv[++i], opts->gen)) return -1;
		
		else if(strcmp(argv[i] ,"--nrefs") == 0)
			if(!parseLong(argv[++i], &opts->nrefs)) return -1;
		
		else if(strcmp(argv[i] ,"--npages") == 0)
                         if(!parseInt(argv[++i], &opts->npages)) return -1;

		else if(strcmp(argv[i] ,"--frames") == 0)
                         if(!parseInt(argv[++i], &opts->frames)) return -1;

		else if(strcmp(argv[i] ,"--pwrite") == 0)
                         if(!parseDouble(argv[++i], &opts->pwrite)) return -1;
				
		else if(strcmp(argv[i] ,"--seed") == 0)
                         if(!parseInt(argv[++i], &opts->seed)) return -1;
		
		else if(strcmp(argv[i] ,"--out") == 0)
                         if(!parseOut(argv[++i], opts->out)) return -1;
				
		else if(strcmp(argv[i] ,"--help") == 0)
			return -1;
		
   		else
			fprintf(stderr, "command error: can't recognize this %s", argv[i]);
			return -1;
		
	}
	return 0;
}

static void usage(const char *prog)
{
	fprintf(stderr, "Usage: %s --selftest\n", prog);
	fprintf(stderr, "	%s --gen random|locality|mine --npages N --frames N --pwrite P(0<=P<=1) --seed S --out stdout(default)\n", prog);
}

static void selftest(const Options *opts, const Ref *refs)
{
	
    	/*int seq[] = {7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1};
    	for (long i = 0; i < n; i++) 
	{
		refs[i].page = seq[i] + 1; 
		refs[i].is_write = 0; 
	}
	*/
	printf("Reference generate pattern: %s\n", opts->gen);

	// FIFO
	printf("FIFO:\n");
    	FifoState fs = {0};
   	Policy fifo = { "FIFO", NULL, fifoVictim, &fs };
    	Stats fifo_stats = simulate(opts->frames, opts->nrefs, refs, &fifo);
   	printf("[selftest] %s, faults=%ld, disk_writes=%d, interrupts=%d\n\n",fifo.name, fifo_stats.faults, fifo_stats.disk_writes, fifo_stats.interrupts);

	// ESC
	printf("ESC:\n");
	ESCState esc_s = {0};
        Policy esc = { "ESC", escHit, escVictim, &esc_s };
        Stats esc_stats = simulate(opts->frames, opts->nrefs, refs, &esc);
	printf("[selftest] %s, faults=%ld, disk_writes=%d, interrupts=%d\n\n",fifo.name, fifo_stats.faults, fifo_stats.disk_writes, fifo_stats.interrupts

	// MFU
	printf("MFU:\n");
	Policy mfu = { "MFU", mfuHit, mfuVictim, NULL};
	Stats mfu_stats = simulate(opts->frames, opts->nrefs, refs, &mfu);
	printf("[selftest] %s, faults=%ld, disk_writes=%d, interrupts=%d\n\n",fifo.name, fifo_stats.faults, fifo_stats.disk_writes, fifo_stats.interrupts
}


int main(int argc, char *argv[])
{
	Options *opts;
	optionsInit(opts);

	if (argc >= 2 && strcmp(argv[1], "--selftest") == 0) 
	{
		if(parseArgs(argc, argv, opts) < 0)
			usage(argv[0])
			return 1;
		selftest(opts)
		return 0;
	}
}
