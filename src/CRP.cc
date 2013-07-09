#include "CRP.h"

#include "util.h"
#include "rand.h"

void CRP::Hyper::resamp() {
    R gamma1 = 1, gamma2 = 1, beta1 = 1, beta2 = 1;
    LOG("resampling h with %d CRPs:\n", (N) deps.size());
    for(const CRP *p : deps) { const CRP &cr = *p;
        FOR(i, 1,cr.t)
            if(flip(a / (a + d*i))) gamma1 += 1;
            else                     beta1 += 1;
        FOR_VAL(rm, cr.rs) FOR_PAIR(s,n, rm) FOR(n) FOR(j, 1,s)
            beta2 += flip((1 - d) / (j - d));
        if(cr.c > 1) gamma2 -= log(rand_beta(a + 1, cr.c - 1));
    }
    a = rand_gamma(gamma1, gamma2); d = rand_beta(beta1, beta2);
    LOG("\ta = %g, d = %g\n", a, d);
}



bool CRP::Room::add(R d, R p_new) {
    if(c > 0) SAMPLE(c - d * t + p_new) FOR_PAIR(s,n, SELF)
        WITH_PROB(n * pos(s - d)) return inc(s);
    return inc(0); // new table w.p. propto p_new
}

int CRP::Room::del() {
    SAMPLE(c) FOR_PAIR(s,n, SELF)
        WITH_PROB(n * s) return dec(s);
    assert(false); // shouldn't reach here
}



CRP::CRP(Hyper &hyper, Exch &parent)
        : h(hyper), par(parent), t(0), c(0) {
    hyper.reg(this);
}

R CRP::pred(X x) const {
    if(c == 0) return par.pred(x);
    R p = (h.a + h.d * t) * par.pred(x);
    if(rs.count(x)) p += rs.at(x).c - h.d * rs.at(x).t;
    return p / (h.a + c);
}

X CRP::samp() const {
    SAMPLE(h.a + c) FOR_PAIR(x,rm, rs)
        WITH_PROB(pos(rm.c - h.d * rm.t)) return x;
    return par.samp(); // w.p. propto h.a + h.d * t
}

void CRP::add(X x) {
    R p_new = (h.a + h.d * t) * par.pred(x);
    c += 1; if(rs[x].add(h.d, p_new)) { t += 1; par.add(x); }
}

void CRP::del(X x) {
    c -= 1; if(rs[x].del()) { t -= 1; par.del(x); }
}

void CRP::resamp() {
    FOR_PAIR(x,rm, rs) FOR(rm.c) { del(x); add(x); }
}
