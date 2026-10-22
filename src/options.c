#include <stddef.h>
#include "options.h"

/* Defaults = the assignment's minimum (150,000 refs, pages 1..1500). */
Options defaultOptions(void)
{
	return (Options){
		.selftest = 0,
		.gen = "random",
		.nrefs = 150000,
		.npages = 1500,
		.frames = 0,
		.pwrite = 0.3,
		.seed = 1,
		.out = NULL
	};
}
