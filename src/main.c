//#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "refgen.h"

int main(int argc, char *argv[])
{
	//if (strcmp(argv[1], "random") == 0)
	//{}
	long n = 150000;
	Ref *ref_p = malloc(n * sizeof(Ref));
	if (ref_p == NULL) {exit(1);}
	genRandom(ref_p, n, 1500, 0.3, 1);

	int j;
	for(j=0; j<30; j++)
		printf("out[%d]:{page: %d, is_write: %d}\n", j, ref_p[j].page, ref_p[j].is_write);

	return 0;
}
