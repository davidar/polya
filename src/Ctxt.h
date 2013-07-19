#pragma once

#include <algorithm>
#include <bitset>

#include "util.h"

template <N L>
class Ctxt {
    X u[L];

    public:
    Ctxt() { FOR(i,L) u[i] = X_NULL; }
    Ctxt(const Ctxt<L> &o) { FOR(i,L) u[i] = o.u[i]; }
    template <class I>
    Ctxt(I a, I b) : Ctxt() { std::copy(a,b, u + L-(b-a)); }

    const X &at(N i) const DO(u[i])
    const Ctxt<L> set(N i, X x = X_NULL) const
        DO(Ctxt<L> v(SELF), v.u[i] = x, v)
    bool operator<(const Ctxt<L> &o) const
        DO(std::lexicographical_compare(u,u+L, o.u,o.u+L))
    N mask() const DO(std::bitset<L> m,
        FOR(i,L) if(u[i] != X_NULL) m.set(i), m.to_ulong())
};
