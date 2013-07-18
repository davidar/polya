#include "CRP.h"

#include "util.h"
#include "rand.h"

#ifdef SLICESAMP
R CRP::Hyper::SampA::f(R a) const {
    if(a <= -h.d) return log(0);
    return log(h.post(a, h.d, false));
}

void CRP::Hyper::SampD::init() {
    m.clear();
    for(auto cr : h.deps) FOR_VAL(rm, cr->rs) FOR_PAIR(s,n, rm.m)
        if(s > 1) m[s] += n;
}

R CRP::Hyper::SampD::f(R d) const {
    if(d <= 0 || 1 <= d) return log(0);
    // the following is a faster version of
    //   log(h.post(h.a, d, true))
    LogR p = h.post(h.a, d, false);
    FOR_PAIR(s,n, m)
        p *= pow(lpoch(1 - d, s - 1), n);
    return log(p);
}
#endif

void CRP::Hyper::resamp() {
    LOG("resamp. CRP::Hyper with %u CRPs:", (N) deps.size());
#ifdef SLICESAMP
    d = samp_d(d); a = samp_a(a);
#else
    R gamma1 = 1, gamma2 = 1, beta1 = 1, beta2 = 1;
    for(auto p : deps) LET(const CRP &cr = *p) {
        FOR(i, 1,cr.t)
            if(flip(a, d*i)) gamma1 += 1;
            else              beta1 += 1;
        FOR_VAL(rm, cr.rs) FOR_PAIR(s,n, rm.m) FOR(n) FOR(j, 1,s)
            beta2 += flip((1 - d) / (j - d));
        if(cr.c > 1) gamma2 -= log(rand_beta(a + 1, cr.c - 1));
    }
    a = rand_gamma(gamma1, gamma2); d = rand_beta(beta1, beta2);
#endif
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
    R p_new = (c > 0) ? (h.a + h.d * t) * par(x) : 1;
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

LogR CRP::l(R a, R d, bool full) const {
    if(c == 0) return LogR(1);
    LogR p = lpoch(a + d, t - 1, d) / lpoch(a + 1, c - 1);
    if(full) FOR_VAL(rm,rs) FOR_PAIR(s,n, rm.m)
        if(s > 1) p *= pow(lpoch(1 - d, s - 1), n);
    return p;
}

void CRP::ser(FILE *f) const {
    fprintf(f, "CRP(%u@%u)\n", c,t);
    FOR_PAIR(x,rm, rs) { fprintf(f, "\t[%u]\t", x); rm.ser(f); }
}
