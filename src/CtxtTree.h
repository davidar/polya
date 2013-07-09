#pragma once

#include "util.h"
#include "CRP.h"
#include "XMap.h"

class CtxtTree {
    CRP r; // node
    XMap<CtxtTree*> c; // children

    public:
    std::vector<CRP::Hyper> &h;
    const N m; // depth

    CtxtTree(Exch &parent, std::vector<CRP::Hyper> &hyper, N depth = 0)
        : h(hyper), m(depth), r(hyper[depth], parent) {}

    CtxtTree &operator[](X x) {
        if(!c.count(x)) c[x] = new CtxtTree(r, h, m+1);
        return *c[x];
    }

    const CtxtTree &at(X x) const DO(*c.at(x))
    bool contains(X x) const DO(c.count(x))

    template <class I>
    CtxtTree &context_raw(I a, I b) {
        if(a == b) return SELF;
        return SELF[*(b-1)].context_raw(a,b-1);
    }
    template <class I>
    CRP &context(I a, I b) DO(context_raw(a,b).r)

    template <class I>
    const CtxtTree &context_at_raw(I a, I b) const {
        if(a == b || !contains(*(b-1))) return SELF;
        return at(*(b-1)).context_at_raw(a,b-1);
    }
    template <class I>
    const CRP &context_at(I a, I b) const DO(context_at_raw(a,b).r)

    void resamp() { r.resamp(); FOR_VAL(v,c) v->resamp(); }
};
