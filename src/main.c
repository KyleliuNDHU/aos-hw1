#include <string.h>
#include <stdio.h>
//#include <stdlib.h>
#include "refgen.h"
#include "policy.h"
#include "sim.h"

static void usage(const char *prog)
{
	fprintf(stderr, "Usage: %s --selftest\n", prog);
	fprintf(stderr, "	%s --gen random|locality|mine --frames N --seed S\n", prog);
}

static void selftest(void)
{
    	int seq[] = {7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1};
	long n = sizeof(seq) / sizeof(seq[0]);
    	Ref refs[20];

    	for (long i = 0; i < n; i++) 
	{
		refs[i].page = seq[i] + 1; 
		refs[i].is_write = 0; 
	}

	// FIFO
	printf("FIFO:\n");
    	FifoState fs = {0};
   	Policy fifo = { "FIFO", NULL, fifoVictim, &fs };
    	Stats fifo_stats = simulate(3, n, refs, &fifo);
   	printf("[selftest] %s faults=%ld (expect 15)\n\n",fifo.name, fifo_stats.faults);

	// ESC
	printf("ESC:\n");
	ESCState esc_s = {0};
        Policy esc = { "ESC", escHit, escVictim, &esc_s };
        Stats esc_stats = simulate(3, n, refs, &esc);
        printf("[selftest] %s faults=%ld (expect 14)\n\n", esc.name, esc_stats.faults);
	
	// MFU
	printf("MFU:\n");
	Policy mfu = { "MFU", mfuHit, mfuVictim, NULL};
	Stats mfu_stats = simulate(3, n, refs, &mfu);
	printf("[selftest] %s faults=%ld (expect 15)\n", mfu.name, mfu_stats.faults);

}


int main(int argc, char *argv[])
{
	if (argc >= 2 && strcmp(argv[1], "--selftest") == 0) 
	{
		selftest();	
		return 0;
	}
	usage(argv[0]);
	return 1;
}
