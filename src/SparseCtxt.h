#pragma once

#include <map>
#include <vector>

#include "util.h"
#include "Ctxt.h"
#include "CRP.h"
#include "Mixture.h"
#include "Weights.h"

template <N L>
class SparseCtxt {
    std::vector<CRP::Hyper> &hr;
    std::map<Ctxt<L>, CRP *> cs;
    std::map<Ctxt<L>, Weights::Weight> ws;
    std::vector<Mixture *> mixs;

    Exch &base(const Ctxt<L> u) {
        std::vector<Exch *> cpts;
        std::vector<Weights::Weight *> weights;
        FOR(i,L) if(u.at(i) != X_NULL) LET(auto v = u.set(i))
            cpts.push_back(&SELF[v]), weights.push_back(&ws[v]);
        if(cpts.size() == 1) return *(cpts[0]);

        Mixture *mix = new Mixture(*new Weights(weights), cpts);
        mixs.push_back(mix);
        return *mix;
    }

    public:
    SparseCtxt(std::vector<CRP::Hyper> &hyper, Exch &parent)
        : hr(hyper) { cs[Ctxt<L>()] = new CRP(hr[0], parent); }

    CRP &operator[](const Ctxt<L> u) {
        if(!cs.count(u))
            cs[u] = new CRP(hr[u.mask()], base(u));
        return *cs[u];
    }

    void resamp() {
        FOR_VAL(cr,cs) cr->resamp();
        FOR_VAL(w,ws) w.resamp();
        for(auto mix : mixs) mix->resamp();
    }
};
