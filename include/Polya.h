#pragma once

#include <vector>
#include <unordered_map>
#include <sstream>
#include <iostream>

#include "util.h"
#include "rand.h"
#include "Exchangeable.h"
#include "Partition.h"
#include "LogR.h"
#include "SliceSamp.h"

class Polya : public Exchangeable {
    public:
    class Hyper {
        const std::string name;
        std::vector<const Polya *> deps;

        class SampA : public SliceSamp {
            const Hyper &h;

            public:
            SampA(const Hyper &par) : h(par) {}
            LogR f(R a) const {
                if(a <= -h.d) return log(0);
                return h.posterior(a, h.d, false);
            }
        } samp_a;

        class SampD : public SliceSamp {
            const Hyper &h;
            std::unordered_map<N,N> m;

            public:
            SampD(const Hyper &par) : h(par) {}
            void init() {
                m.clear();
                for(auto cr : h.deps) FOR_VAL(rm, cr->rs) FOR_PAIR(s,n, rm.m)
                    if(s > 1) m[s] += n;
            }
            LogR f(R d) const {
                if(d <= 0 || 1 <= d) return log(0);
                // the following is a faster version of
                //   log(h.posterior(h.a, d, true))
                LogR p = h.posterior(h.a, d, false);
                FOR_PAIR(s,n, m)
                    p *= pow(lpoch(1 - d, s - 1), n);
                return p;
            }
        } samp_d;

        LogR posterior(R a, R d, bool full) const {
            LogR l(1);
            for(auto cr : deps) l *= cr->likelihood(a, d, full);
            return l / (a + d);
        }

        public:
        R a, d; // concentration, discount

        Hyper(std::string s)
            : name(s), samp_a(SELF), samp_d(SELF), a(1.0), d(0.5) {}
        Hyper() : Hyper("") {}

        void registerDep(const Polya *crp) { deps.push_back(crp); }
        void resample() {
            LOG("'%s' hyper-resample (%u):", name.c_str(), (N) deps.size());
            assert(deps.size() > 0); // unused hyper likely indicates bug
            d = samp_d(d); a = samp_a(a);
            LOG("\ta = %g, d = %g", a, d);
        }
    };

    private:
    class Room : public Partition {
        public:
        bool add(R d, R p_new) {
            ASSERT(,p_new, >= 0);
            if(c > 0) SAMPLE(c - d * t + p_new) FOR_PAIR(s,n, m)
                WITH_PROB(n * pos(s - d)) return inc(s);
            return inc(0); // new table w.p. propto p_new
        }
        bool del() {
            ASSERT(0 < ,t, && t <= ,c,);
            SAMPLE(c) FOR_PAIR(s,n, m)
                WITH_PROB(n * s) return dec(s);
            // shouldn't reach here
            std::cerr << toString(); assert(false);
        }

        std::string toString() const {
            std::stringstream ss;
            ss << "Room(" << c << "@" << t << "):";
            FOR_PAIR(s,n, m) FOR(n) ss << " " << s;
            ss << "\n";
            return ss.str();
        }
    };

    Hyper &h;
    Exchangeable &par;
    std::unordered_map<X,Room> rs; // map x to room of tables

    void add(X x, Room &rm) {
        R p_new = (c > 0) ? (h.a + h.d * t) * par(x) : 1;
        c += 1; if(rm.add(h.d, p_new)) { t += 1; par += x; }
    }
    void del(X x, Room &rm) {
        c -= 1; if(rm.del()) { t -= 1; par -= x; }
    }

    public:
    N t, c; // total number of tables and customers

    Polya(Hyper &hyper, Exchangeable &parent)
        : h(hyper), par(parent), t(0), c(0) { h.registerDep(this); }

    R predict(X x) const {
        if(c == 0) return par(x);
        R p = (h.a + h.d * t) * par(x);
        IF_FIND(x,rm, rs) p += rm.c - h.d * rm.t;
        ASSERT(0 <= ,p, && p <= ,h.a + c,);
        return p / (h.a + c);
    }

    X sample() const {
        SAMPLE(h.a + c) FOR_PAIR(x,rm, rs)
            WITH_PROB(pos(rm.c - h.d * rm.t)) return x;
        return par(); // w.p. propto h.a + h.d * t
    }

    Exchangeable &observe(X x) DO(add(x,rs[x]), SELF)
    Exchangeable &forget(X x) DO(del(x,rs[x]), SELF)

    void resample() {
        FOR_PAIR(x,rm, rs) FOR(rm.c) { del(x,rm); add(x,rm); }
    }

    LogR likelihood(R a, R d, bool full = true) const { 
        if(c == 0) return LogR(1);
        LogR p = lpoch(a + d, t - 1, d) / lpoch(a + 1, c - 1);
        if(full) FOR_VAL(rm,rs) FOR_PAIR(s,n, rm.m)
            if(s > 1) p *= pow(lpoch(1 - d, s - 1), n);
        return p;
    }

    std::string toString() const {
        std::stringstream ss;
        ss << "Polya(" << c << "@" << t << ")\n";
        FOR_PAIR(x,rm, rs) { ss << "\t[" << x << "]\t" << rm.toString(); }
        return ss.str();
    }
};
