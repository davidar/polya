#include "CRP.h"

#include "util.h"
#include "rand.h"

void CRP::Hyper::resamp() {
    R gamma1 = 1, gamma2 = 1, beta1 = 1, beta2 = 1;
    LOG("resampling h with %d CRPs:", (N) deps.size());
    for(const CRP *p : deps) { const CRP &cr = *p;
        FOR(i, 1,cr.t)
            if(flip(a / (a + d*i))) gamma1 += 1;
            else                     beta1 += 1;
        FOR_VAL(rm, cr.rs) FOR_PAIR(s,n, rm) FOR(n) FOR(j, 1,s)
            beta2 += flip((1 - d) / (j - d));
        if(cr.c > 1) gamma2 -= log(rand_beta(a + 1, cr.c - 1));
    }
    a = rand_gamma(gamma1, gamma2); d = rand_beta(beta1, beta2);
    LOG("\ta = %g, d = %g", a, d);
}



bool CRP::Room::add(R d, R p_new) { ASSERT(,p_new, >= 0);
    if(c > 0) SAMPLE(c - d * t + p_new) FOR_PAIR(s,n, SELF)
        WITH_PROB(n * pos(s - d)) return inc(s);
    return inc(0); // new table w.p. propto p_new
}

int CRP::Room::del() {
    ASSERT(0 < ,t, && t <= ,c,);
    SAMPLE(c) FOR_PAIR(s,n, SELF)
        WITH_PROB(n * s) return dec(s);
    DBG("c = %u", c); assert(false); // shouldn't reach here
}

void CRP::Room::ser(FILE *f) const {
    fprintf(f, "Room(%u@%u):", c,t);
    FOR_PAIR(s,n, SELF) FOR(n) fprintf(f, " %u", s);
    fprintf(f, "\n");
}



CRP::CRP(Hyper &hyper, Exch &parent)
        : h(hyper), par(parent), t(0), c(0) {
    hyper.reg(this);
}

R CRP::operator()(X x) const {
    if(c == 0) return par(x);
    R p = (h.a + h.d * t) * par(x);
    if(rs.count(x)) p += rs.at(x).c - h.d * rs.at(x).t;
    ASSERT(0 <= ,p, && p <= ,h.a + c,);
    return p / (h.a + c);
}

X CRP::operator()() const {
    SAMPLE(h.a + c) FOR_PAIR(x,rm, rs)
        WITH_PROB(pos(rm.c - h.d * rm.t)) return x;
    return par(); // w.p. propto h.a + h.d * t
}

Exch &CRP::operator+=(X x) {
    R p_new = (h.a + h.d * t) * par(x);
    c += 1; if(rs[x].add(h.d, p_new)) { t += 1; par += x; }
    return SELF;
}

Exch &CRP::operator-=(X x) {
    c -= 1; if(rs[x].del()) { t -= 1; par -= x; }
    return SELF;
}

void CRP::resamp() {
    FOR_PAIR(x,rm, rs) FOR(rm.c) (SELF -= x) += x;
#ifndef NDEBUG
    N ntab = 0, ncust = 0;
    FOR_VAL(rm,rs) { rm.check(); ntab += rm.t; ncust += rm.c; }
    ASSERT(,t, == ,ntab,); ASSERT(,c, == ,ncust,);
#endif
}

void CRP::ser(FILE *f) const {
    fprintf(f, "CRP(%u@%u)\n", c,t);
    FOR_PAIR(x,rm, rs) { fprintf(f, "\t[%u]\t", x); rm.ser(f); }
}
