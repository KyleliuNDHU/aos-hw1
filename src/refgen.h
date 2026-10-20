#ifndef REFGEN_H
#define REFGEN_H
	
typedef struct {int page; int is_write;}Ref;

void genRandom	(Ref *out, long n, int npages, double prob_write, unsigned seed);
void genLocality(Ref *out, long n, int npages, double prob_write, unsigned seed);
void genMine	(Ref *out, long n, int npages, double prob_write, unsigned seed);

#endif
