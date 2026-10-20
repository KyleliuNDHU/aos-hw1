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

    	FifoState fs = {0};
   	Policy fifo = { "FIFO", NULL, fifoVictim, &fs };
    	Stats s = simulate(3, n, refs, &fifo);
   	printf("[selftest] FIFO faults=%ld (expect 15)\n", s.faults);
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
