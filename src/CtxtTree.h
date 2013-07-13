#pragma once

#include "util.h"
#include "CRP.h"
#include "XMap.h"

class CtxtTree {
    CRP r; // node
    XMap<CtxtTree*> c; // children

    template <class I>
    CtxtTree &context_raw(I a, I b) {
        if(a == b) return SELF;
        return SELF[*(b-1)].context_raw(a,b-1);
    }

    template <class I>
    const CtxtTree &context_at_raw(I a, I b) const {
        if(a == b) return SELF;
        IF_FIND(*(b-1),ct, c)
            return ct->context_at_raw(a,b-1);
        return SELF;
    }

    public:
    std::vector<CRP::Hyper> &h;
    const N m; // depth

    CtxtTree(Exch &parent, std::vector<CRP::Hyper> &hyper, N depth = 0)
        : h(hyper), m(depth), r(hyper[depth], parent) {}

    bool contains(X x) const DO(c.count(x))
    CtxtTree &operator[](X x)
        DO(if(!contains(x)) c[x] = new CtxtTree(r,h,m+1), *c[x])
    const CtxtTree &at(X x) const DO(*c.at(x))
    template <class I> CRP &context(I a, I b) DO(context_raw(a,b).r)
    template <class I> const CRP &context_at(I a, I b) const
        DO(context_at_raw(a,b).r)
    void resamp() { r.resamp(); FOR_VAL(v,c) v->resamp(); }
};
