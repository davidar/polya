#pragma once

#include <stdlib.h>

#include <random>

#include "util.h"

static std::default_random_engine rng;

inline R randu(R a = 0, R b = 1) { // U(a,b)
    assert(a < b);
    return a + (b-a) * (R) rand() / RAND_MAX;
}

// SAMPLE(z) for(...) {... WITH_PROB(p) {...} ...} is equivalent to
//   R r = randu(0,z); for(...) {... r -= p; if(r <= 0) {...} ...}
// where the body of WITH_PROB should end with a break or return statement
inline R _check_nonneg(R x) DO(assert(x >= 0), x)
#define SAMPLE(z) LET(R _samp_r = randu(0,_check_nonneg((z))))
#define WITH_PROB(p) if((_samp_r -= _check_nonneg((p))) <= 0)

inline bool flip(R p) { // Bernoulli(p)
    if(p == 0) return false;
    if(p == 1) return true;
    return (randu() < p);
}

inline R rand_gamma(R a, R l) { // Gamma(a,l)
    return std::gamma_distribution<R>(a,1/l)(rng);
}

inline R rand_beta(R a, R b) { // Beta(a,b)
    R x = rand_gamma(a,1), y = rand_gamma(b,1);
    return x / (x + y);
}
