#pragma once

#include <vector>

#include "util.h"
#include "Exchangeable.h"
#include "DynDict.h"

class LM : public Exchangeable {
    typedef std::vector<X> W;
    typedef DynDict<W,Exchangeable*> G;
    std::vector<W> dict; // id -> seq
    const N L;
    G contextual_dist;
    
    Exchangeable &c(const W& w, N t) const {
        N start = (t < L) ? 0 : t-L;
        W u(w.begin() + start, w.begin() + t);
        return *(contextual_dist[u]);
    }
    
    public:
    LM(N context_len, G::F f, void *user_data)
        : L(context_len), contextual_dist(f,user_data) {}
    
    X add_to_dict(W w) {
        dict.push_back(w);
        return dict.size() - 1;
    }
    
    R predict(N i) const {
        const W& w = dict[i];
        R p = 1;
        FOR(t, w.size())
            p *= c(w,t).predict(w[t]);
        return p;
    }
    
    R predict_log(N i) const {
        const W& w = dict[i];
        LogR p(1);
        FOR(t, w.size())
            p *= c(w,t).predict(w[t]);
        return log(p);
    }
    
    X sample() const {
        assert(0); // TODO
    }
    
    Exchangeable &observe(N i) {
        const W& w = dict[i];
        FOR(t, w.size()) c(w,t).observe(w[t]);
        return SELF;
    }
    
    Exchangeable &forget(N i) {
        const W& w = dict[i];
        FOR(t, w.size()) c(w,t).forget(w[t]);
        return SELF;
    }
    
    void resample() {}
    
    std::string toString() const {
        return "LM";
    }
};
