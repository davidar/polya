#pragma once

#include <vector>

#include "util.h"
#include "rand.h"
#include "Exch.h"
#include "XMap.h"

#define MIXTURE_MAX 32

class Mixture : public Exch {
    Exch &pi; // component weights
    const std::vector<Exch *> c; // components
    const N k; // number of components
    std::vector<XMap<N>> ns; XMap<N> n; // observation counts

    R pred(X x, N i) const DO(Exch *e = c.at(i), pi(i+1) * (*e)(x))
    Exch &add(X x, N i) DO(pi += i+1, *(c[i]) += x, ns[i][x]++, n[x]++, SELF)
    Exch &del(X x, N i) DO(pi -= i+1, *(c[i]) -= x, ns[i][x]--, n[x]--, SELF)

    public:
    Mixture(Exch &weights, const std::vector<Exch *> cpts)
        : pi(weights), c(cpts), k(c.size()), ns(k) {}

    R operator()(X x) const DO(R p = 0, FOR(i,k) p += pred(x,i), p)
    X operator()() const DO(Exch *e = c[pi()-1], (*e)())

    Exch &operator+=(X x) {
        R p = 0; R w[MIXTURE_MAX]; FOR(i,k) p += w[i] = pred(x,i);
        SAMPLE(p) FOR(i,k)
            WITH_PROB(w[i]) return add(x,i);
        assert(false);
    }

    Exch &operator-=(X x) {
        SAMPLE(n[x]) FOR(i,k)
            WITH_PROB(ns[i][x]) return del(x,i);
        assert(false);
    }

    void resamp() {
        FOR_PAIR(x,nx, n) FOR(nx) (SELF -= x) += x;
    }

    void ser(FILE *f) const {
        fprintf(f, "Mixture(%u)\n", k);
        fprintf(f, "pi: "); pi.ser(f);
        FOR(i,k) fprintf(f, "c%u: ", i+1), c[i]->ser(f);
    }
};
