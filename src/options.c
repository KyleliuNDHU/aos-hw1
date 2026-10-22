#include <stddef.h>
#include "options.h"

Options defaultOptions()
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
