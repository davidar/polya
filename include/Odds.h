#pragma once

#include <vector>
#include <sstream>

#include "util.h"
#include "Exchangeable.h"
#include "SliceSamp.h"
#include "LogR.h"

class Odds : public Exchangeable {
    public:
    class Term {
        R t;

        struct : public SliceSamp {
            std::vector<std::pair<const Odds *, N>> deps;
            LogR f(R t) const {
                if(t <= 0) return log(0);
                LogR p(1);
                FOR_PAIR(odds,i, deps)
                    p *= odds->likelihood(i,t);
                return p / t;
            }
        } samp_t;

        public:
        Term() : t(1.0) {}
        operator R() const DO(t)
        void registerDep(const Odds *odds, N pos)
            { samp_t.deps.push_back(std::make_pair(odds, pos)); }
        void resample() { t = samp_t(t); }
    };

    private:
    const std::vector<Term *> ts;
    const N k;
    std::vector<N> ns; // observation counts
    N n; // total observations

    R sum() const DO(R s = 0, for(auto t : ts) s += *t, s)

    public:
    Odds(const std::vector<Term *> terms)
        : ts(terms), k(ts.size()), ns(k), n(0) {
        FOR(i,k) ts.at(i)->registerDep(this, i);
    }

    R predict(X x) const DO(*ts.at(x) / sum())

    X sample() const {
        SAMPLE(sum()) FOR(i,k)
            WITH_PROB(*ts.at(i)) return i;
        assert(false);
    }

    Exchangeable &observe(X x) DO(ns[x] += 1, n += 1, SELF)
    Exchangeable &forget(X x)  DO(ns[x] -= 1, n -= 1, SELF)

    void resample() {}
    std::string toString() const {
        std::stringstream ss;
        ss << "Odds(" << (R) *ts[0];
        FOR(i,1,k) ss << ":" << (R) *ts[i];
        ss << ")";
        return ss.str();
    }

    LogR likelihood(N i, R t) const {
        if(n == 0) return LogR(1);
        R sum_new = sum() - *ts.at(i) + t;
        return lpow(t, ns[i]) / lpow(sum_new, n);
    }
};
