#include <stdlib.h>
#include "refgen.h"

void genRandom(Ref *out, long n, int npages, double pwrite, unsigned seed)
{
	srand(seed);

	long i;
	for(i=0; i<n; i++)
	{
		// rand() % npages : 0 to (npages-1),
		// but the range of ref. start from 1,
		// so add 1 to let it be 1 to napges.
		out[i].page = 1 + rand() % npages;

		/**
		 * The range of integer which returned by rand() is 0 to RAND_MAX,
		 * and also uniformly distributed in this range.
		 * So rand() / RAND_MAX can get a uniform random probability
		 * between [0, 1].
		 * Use (double) for type casting avoiding the loss of integer division.
		 */
		double uniform_random_prob = (double) rand() / RAND_MAX;
		// is_write: pwrite % set 1; (1-pwrite) % set 0.
		out[i].is_write = (uniform_random_prob < pwrite);
	}
}

void genLocality(Ref *out, long n, int npages, double pwrite, unsigned seed)
{
	// Test constants : Suppose n = 1500000
	// CALL_LEN: the reference string length of procedure call.
	const int CALL_LEN_MIN = 750;
	const int CALL_LEN_MAX = 1500;
	// WIND: the range of reference strings, the procedure call access.
	const int WIND_MIN = 20;
	const int WIND_MAX = 50;
	// Test constants : ends

	srand(seed);

	long filled = 0; // the number of out already be filled

	while(filled < n)
	{	/**
		 * CALL_LEN_MAX - CALL_LEN_MIN : the offset range
		 * +1: because 0 (base) is also a legal offset here
		 * rand() % (CALL_LEN_MAX - CALL_LEN_MIN + 1): choose a offset here
		 * final formula: base + offset = the random reference length of the procedure call
		 */
		int ref_len = CALL_LEN_MIN + rand() % (CALL_LEN_MAX - CALL_LEN_MIN + 1);

		// simillar reasons above.
		int wind_width = WIND_MIN + rand() % (WIND_MAX - WIND_MIN + 1);
		int start = 1 + rand() % (npages - wind_width + 1);// the range of ref. can't beyond the npages

		int j;
		for(j=0; j<ref_len; j++)
		{
			if(filled >= n) break; // if already filled n references means accomplish the requirement.
			out[filled].page = start + rand() % wind_width;
			// Suppose the procedure never access the pages out of its windows starts from the int. variable start stored.


			double uniform_random_prob = (double) rand()/RAND_MAX;
			out[filled].is_write = (uniform_random_prob < pwrite);

			filled++;
		}
	}
}

void genMine(Ref *out, long n, int npages, double pwrite, unsigned seed)
{
	genRandom(out, n, npages, pwrite, seed); /*TODO: placeholder */
}
