#include "options.h"

void optionsInit(Options *opts)
{
	opts -> selftest = 0;
	opts -> gen = "random";
	opts -> nrefs = 150000; 
	opts -> npages = 1500;	
	opts -> frames = 0;	
	opts -> pwrite = 0.3;
	opts -> seed = 1;
	opts -> out = "stdout";
}
