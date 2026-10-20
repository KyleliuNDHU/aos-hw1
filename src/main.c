//#include <string.h>
#include <stdio.h>
//#include <stdlib.h>
#include "refgen.h"
#include "policy.h"
#include "sim.h"

#ifdef DEBUG
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
        printf("[selftest] %s faults=%ld (expect 12)\n\n", esc.name, esc_stats.faults);
	
	// MFU
	printf("MFU:\n");
	Policy mfu = { "MFU", mfuHit, mfuVictim, NULL};
	Stats mfu_stats = simulate(3, n, refs, NULL);
	printf("[selftest] %s faults=%ld (expect 14)\n", mfu.name, mfu_stats.faults);

}
#endif


int main(int argc, char *argv[])
{
	selftest();
	
	//if (strcmp(argv[1], "random") == 0)
	//{}
	// long n = 150000;
	// Ref *ref_p = malloc(n * sizeof(Ref));
	//if (ref_p == NULL) {exit(1);}
	//genRandom(ref_p, n, 1500, 0.3, 1);

	//int j;
	//for(j=0; j<30; j++)
	//	printf("out[%d]:{page: %d, is_write: %d}\n", j, ref_p[j].page, ref_p[j].is_write);

	return 0;
}
