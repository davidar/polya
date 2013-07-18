#pragma once

#include <vector>
#include <unordered_map>

#include "util.h"
#include "Exch.h"
#include "XMap.h"
#include "NBag.h"
#include "LogR.h"
#include "SliceSamp.h"

class CRP : public Exch {
    public:
    class Hyper {
        std::vector<const CRP *> deps;

#ifdef SLICESAMP
        class SampA : public SliceSamp {
            const Hyper &h;

            public:
            SampA(const Hyper &par) : h(par) {}
            R f(R a) const;
        } samp_a;

        class SampD : public SliceSamp {
            const Hyper &h;
            std::unordered_map<N,N> m;

            public:
            SampD(const Hyper &par) : h(par) {}
            void init();
            R f(R d) const;
        } samp_d;
#endif

        LogR post(R a, R d, bool full) const DO(LogR l(1),
            for(auto cr : deps) l *= cr->l(a, d, full), l / (a + d))

        public:
        R a, d; // concentration, discount

#ifdef SLICESAMP
        Hyper() : a(1.0), d(0.5), samp_a(SELF), samp_d(SELF) {}
#else
        Hyper() : a(1.0), d(0.5) {}
#endif

        void reg(const CRP *crp) { deps.push_back(crp); }
        void resamp();
    };

    private:
    class Room : public NBag {
        public:
        bool add(R d, R p_new);
        int del();

        void ser(FILE *f) const;
    };

    Hyper &h;
    Exch &par;
    XMap<Room> rs; // map x to room of tables

    void add(X x, Room &rm);
    void del(X x, Room &rm);

    public:
    N t, c; // total number of tables and customers

    CRP(Hyper &hyper, Exch &parent);
    R operator()(X x) const;
    X operator()() const;
    Exch &operator+=(X x) DO(add(x,rs[x]), SELF)
    Exch &operator-=(X x) DO(del(x,rs[x]), SELF)
    void resamp();

    LogR l(R a, R d, bool full = true) const; // (log) likelihood

    void ser(FILE *f) const;
};
