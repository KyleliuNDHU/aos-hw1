#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "refgen.h"

void genRandom(Ref *out, long n, int npages, double pwrite, unsigned seed)
{
	srand(seed);

	long i;
	for(i=0; i<n; i++)
	{
		// rand() % npages : 0 to (npages-1),
		// but the range of ref. start from 1,
		// so add 1 to let it be 1 to npages.
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
	// Tunable constants (chosen for n = 150000, npages = 1500).
	// CALL_LEN: how many references one procedure call makes before it returns.
	const int CALL_LEN_MIN = 750;
	const int CALL_LEN_MAX = 1500;
	// WIND: how many consecutive pages one procedure call touches (its working set).
	// 20~50 straddles the tested frame counts 10..100, so the figures show both
	// "working set fits" and "does not fit".
	const int WIND_MIN = 20;
	const int WIND_MAX = 50;
	// Tunable constants : end

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

		// same idea as above
		int wind_width = WIND_MIN + rand() % (WIND_MAX - WIND_MIN + 1);
		int start = 1 + rand() % (npages - wind_width + 1);// the range of ref. can't beyond the npages

		int j;
		for(j=0; j<ref_len; j++)
		{
			if(filled >= n) break; // if already filled n references means accomplish the requirement.
			out[filled].page = start + rand() % wind_width;
			// a procedure never touches pages outside its own window [start, start+wind_width-1]


			double uniform_random_prob = (double) rand()/RAND_MAX;
			out[filled].is_write = (uniform_random_prob < pwrite);

			filled++;
		}
	}
}

/* ---- genMine: OLTP (Zipf hot/cold) + periodic read-only sequential scan ----
 * Models a database buffer pool. Most references follow a Zipf distribution
 * (a few hot pages take most of the traffic). After every OLTP phase a report
 * query scans SCAN_LEN consecutive pages once; SCAN_LEN is deliberately larger
 * than any frame count (100), so the scan evicts the hot pages ("scan
 * pollution"). The scan region is the upper half of the page range so that it
 * never overlaps the hottest pages, which keeps the effect easy to interpret. */

#define ZIPF_S        1.0     /* skew; 0.8~1.0 is what real workloads measure */
#define OLTP_LEN_MIN  5000    /* references per OLTP phase */
#define OLTP_LEN_MAX  15000
#define SCAN_LEN_MIN  200     /* pages per scan, deliberately > 100 frames */
#define SCAN_LEN_MAX  500

/* cdf[k] = P(rank <= k) for Zipf(s); cdf[0] = 0, cdf[npages] = 1
 *
 * Fill cdf[0..npages] with the cumulative Zipf(s) distribution over ranks
 * 1..npages: cdf[k] = P(rank <= k), cdf[0] = 0, cdf[npages] = 1.
 * h is the normalization constant sum(1/k^s) so the probabilities sum to 1. */
static void buildZipfCdf(double *cdf, int npages, double s)
{
	double h = 0.0;                               /* normalization constant */
	for (int k = 1; k <= npages; k++) h += 1.0 / pow(k, s);
	cdf[0] = 0.0;
	for (int k = 1; k <= npages; k++) cdf[k] = cdf[k-1] + (1.0 / pow(k, s)) / h;
}

/* inverse-CDF sampling: binary search for the first k with cdf[k] >= u */
static int zipfSample(const double *cdf, int npages)
{
	double u = (double)rand() / RAND_MAX;
	int lo = 1, hi = npages;
	while (lo < hi) {
		int mid = (lo + hi) / 2;
		if (cdf[mid] >= u) hi = mid; else lo = mid + 1;
	}
	return lo;
}

void genMine(Ref *out, long n, int npages, double pwrite, unsigned seed)
{
	srand(seed);
	double *cdf = malloc((npages + 1) * sizeof *cdf);
	if (cdf == NULL) { fprintf(stderr, "malloc failed\n"); exit(1); }
	buildZipfCdf(cdf, npages, ZIPF_S);

	long filled = 0;
	int scan = 0;                                 /* 0 = OLTP phase, 1 = scan phase */
	while (filled < n) {
		if (!scan) {
			int len = OLTP_LEN_MIN + rand() % (OLTP_LEN_MAX - OLTP_LEN_MIN + 1);
			for (int j = 0; j < len && filled < n; j++, filled++) {
				out[filled].page = zipfSample(cdf, npages);
				out[filled].is_write = ((double)rand() / RAND_MAX) < pwrite;
			}
		} else {
			int len = SCAN_LEN_MIN + rand() % (SCAN_LEN_MAX - SCAN_LEN_MIN + 1);
			int start = npages / 2 + rand() % (npages / 2 - len + 1);
			for (int j = 0; j < len && filled < n; j++, filled++) {
				out[filled].page = start + j;     /* sequential */
				out[filled].is_write = 0;         /* report query: read-only */
			}
		}
		scan = !scan;
	}
	free(cdf);
}
