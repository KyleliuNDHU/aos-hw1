#include "options.h"

void optionsInit(Options *opts)
{
	opts -> selftest = 0;
	opts -> gen = "random";
	opts -> nrefs = 30; //30 just for test, default : 150000
	opts -> npages = 20;	//20 for test,too;default: 1500
	opts -> frames = 3;	// default = 0
	opts -> pwrite = 0.3;
	opts -> seed = 1;
	opts -> out = NULL;
}
