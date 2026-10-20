#ifndef SIM_H
#define SIM_H

#include "refgen.h"
#include "policy.h"

Stats simulate (int nframes, long n, Ref *refs, Policy *policy);

#endif
