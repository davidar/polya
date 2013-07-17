#pragma once

#include <stdlib.h>

#include <random>

#include "util.h"

static std::default_random_engine rng;

inline R randu(R a = 0, R b = 1) { // U[a,b)
    ASSERT(,a, < ,b,);
    R u = rand() / (1.0 + RAND_MAX);
    return a + (b-a) * u;
}

// SAMPLE(z) for(...) {... WITH_PROB(p) {...} ...} is equivalent to
//   R r = randu(0,z); for(...) {... r -= p; if(r <= 0) {...} ...}
// where the body of WITH_PROB should end with a break or return statement
#define SAMPLE(z) LET(ASSERT(,(z), >= 0)) LET(R _samp_r = randu(0,(z)))
// note that we need to check that p > 0 so that the body of WITH_PROB(0)
// is never executed, even if _samp_r = randu() = 0, in which case the first
// available event is executed no matter what the value of p is
#define WITH_PROB(p) LET(ASSERT(,(p), >= 0)) \
    if((p) > 0 && (_samp_r -= (p)) <= 0)

inline bool flip(R p) { // Bernoulli(p)
    if(p == 0) return false;
    if(p == 1) return true;
    return (randu() < p);
}
inline bool flip(R p, R q) DO(flip(p / (p + q)))

inline R rand_exp(R l = 1) DO(-log(1-randu())/l)

inline R rand_gamma(R a, R l) { // Gamma(a,l)
    return std::gamma_distribution<R>(a,1/l)(rng);
}

inline R rand_beta(R a, R b) { // Beta(a,b)
    R x = rand_gamma(a,1), y = rand_gamma(b,1);
    return x / (x + y);
}
