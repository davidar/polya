#pragma once

#include <vector>

#include "util.h"
#include "Exch.h"
#include "SliceSamp.h"
#include "LogR.h"

class Weights : public Exch {
    public:
    class Weight {
        R w;

        struct : public SliceSamp {
            std::vector<std::pair<const Weights *, N>> deps;
            R f(R w) const {
                if(w <= 0) return log(0);
                LogR p(1); FOR_PAIR(ws,i, deps) p *= ws->l(i,w);
                return log(p) - w;
            }
        } samp_w;

        public:
        Weight() : w(1.0) {}
        operator R() const DO(w)
        void reg(const Weights *ws, N pos)
            { samp_w.deps.push_back(std::make_pair(ws, pos)); }
        void resamp() { w = samp_w(w); }
    };

    private:
    const std::vector<Weight *> ws;
    const N k;
    std::vector<N> ns; // observation counts
    N n; // total observations

    R sum() const DO(R s = 0, for(auto w : ws) s += *w, s)

    public:
    Weights(const std::vector<Weight *> weights)
        : ws(weights), k(ws.size()), ns(k), n(0)
        { FOR(i,k) ws.at(i)->reg(this, i); }

    R operator()(X x) const DO(*ws.at(x-1) / sum())

    X operator()() const {
        SAMPLE(sum()) FOR(i,k)
            WITH_PROB(*ws.at(i)) return 1+i;
        assert(false);
    }

    Exch &operator+=(X x) DO(ns[x-1] += 1, n += 1, SELF)
    Exch &operator-=(X x) DO(ns[x-1] -= 1, n -= 1, SELF)

    void resamp() {}
    void ser(FILE *f) const {/* TODO */}

    LogR l(N i, R w) const {
        if(n == 0) return LogR(1);
        R sum_new = sum() - *ws.at(i) + w;
        return lpow(w, ns[i]) / lpow(sum_new, n);
    }
};
