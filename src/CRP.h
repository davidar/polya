#pragma once

#include <vector>

#include "util.h"
#include "Exch.h"
#include "XMap.h"
#include "NBag.h"

class CRP : public Exch {
    public:
    class Hyper {
        std::vector<const CRP *> deps;

        public:
        R a, d; // concentration, discount

        Hyper() : a(1.0), d(0.5) {}
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

    public:
    N t, c; // total number of tables and customers

    CRP(Hyper &hyper, Exch &parent);
    R operator()(X x) const;
    X operator()() const;
    Exch &operator+=(X x);
    Exch &operator-=(X x);
    void resamp();

    void ser(FILE *f) const;
};
