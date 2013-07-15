#include "CRP.h"

#include "util.h"
#include "rand.h"

void CRP::Hyper::resamp() {
    R gamma1 = 1, gamma2 = 1, beta1 = 1, beta2 = 1;
    LOG("resampling h with %u CRPs:", (N) deps.size());
    for(const CRP *p : deps) { const CRP &cr = *p;
        FOR(i, 1,cr.t)
            if(flip(a / (a + d*i))) gamma1 += 1;
            else                     beta1 += 1;
        FOR_VAL(rm, cr.rs) FOR_PAIR(s,n, rm.m) FOR(n) FOR(j, 1,s)
            beta2 += flip((1 - d) / (j - d));
        if(cr.c > 1) gamma2 -= log(rand_beta(a + 1, cr.c - 1));
    }
    a = rand_gamma(gamma1, gamma2); d = rand_beta(beta1, beta2);
    LOG("\ta = %g, d = %g", a, d);
}



bool CRP::Room::add(R d, R p_new) { ASSERT(,p_new, >= 0);
    if(c > 0) SAMPLE(c - d * t + p_new) FOR_PAIR(s,n, m)
        WITH_PROB(n * pos(s - d)) return inc(s);
    return inc(0); // new table w.p. propto p_new
}

int CRP::Room::del() {
    ASSERT(0 < ,t, && t <= ,c,);
    SAMPLE(c) FOR_PAIR(s,n, m)
        WITH_PROB(n * s) return dec(s);
    ser(stderr); assert(false); // shouldn't reach here
}

void CRP::Room::ser(FILE *f) const {
    fprintf(f, "Room(%u@%u):", c,t);
    FOR_PAIR(s,n, m) FOR(n) fprintf(f, " %u", s);
    fprintf(f, "\n");
}



void CRP::add(X x, Room &rm) {
    R p_new = (h.a + h.d * t) * par(x);
    c += 1; if(rm.add(h.d, p_new)) { t += 1; par += x; }
}

void CRP::del(X x, Room &rm) {
    c -= 1; if(rm.del()) { t -= 1; par -= x; }
}

CRP::CRP(Hyper &hyper, Exch &parent)
    : h(hyper), par(parent), t(0), c(0) { hyper.reg(this); }

R CRP::operator()(X x) const {
    if(c == 0) return par(x);
    R p = (h.a + h.d * t) * par(x);
    IF_FIND(x,rm, rs) p += rm.c - h.d * rm.t;
    ASSERT(0 <= ,p, && p <= ,h.a + c,);
    return p / (h.a + c);
}

X CRP::operator()() const {
    SAMPLE(h.a + c) FOR_PAIR(x,rm, rs)
        WITH_PROB(pos(rm.c - h.d * rm.t)) return x;
    return par(); // w.p. propto h.a + h.d * t
}

void CRP::resamp() {
    FOR_PAIR(x,rm, rs) FOR(rm.c) del(x,rm),add(x,rm);
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
