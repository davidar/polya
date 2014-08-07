#pragma once

#include <vector>
#include <unordered_map>
#include <sstream>

#include "util.h"
#include "rand.h"
#include "Exchangeable.h"

#define MIXTURE_MAX 64

class Mixture : public Exchangeable {
    Exchangeable &pi; // component director
    const std::vector<Exchangeable *> c; // components
    const N k; // number of components
    // observation counts
    std::vector<std::unordered_map<X,N>> ns;
    std::unordered_map<X,N> n;

    R joint(X x, N i) const DO(pi(i) * c[i]->predict(x))

    public:
    Mixture(Exchangeable &p, const std::vector<Exchangeable *> cpts)
        : pi(p), c(cpts), k(c.size()), ns(k) {}

    R predict(X x) const DO(R p = 0, FOR(i,k) p += joint(x,i), p)
    X sample() const DO(c[pi()]->sample())

    Exchangeable &observe(X x) {
        R p = 0; R w[MIXTURE_MAX];
        FOR(i,k) p += w[i] = joint(x,i);
        SAMPLE(p) FOR(i,k)
            WITH_PROB(w[i]) { // add to component i
                pi += i; c[i]->observe(x);
                ns[i][x]++; n[x]++;
                return SELF;
            }
        assert(false); // shouldn't reach here
    }

    Exchangeable &forget(X x) {
        SAMPLE(n[x]) FOR(i,k)
            WITH_PROB(ns[i][x]) { // remove from component i
                pi -= i; c[i]->forget(x);
                ns[i][x]--; n[x]--;
                return SELF;
            }
        assert(false); // shouldn't reach here
    }

    void resample() {
        FOR_PAIR(x,nx, n) FOR(nx) { forget(x); observe(x); }
    }

    std::string toString() const {
        std::stringstream ss;
        ss << "Mixture(" << k << ")\n";
        ss << "pi: " << pi.toString();
        FOR(i,k) ss << "c" << i+1 << ": " << c[i]->toString();
        return ss.str();
    }
};
