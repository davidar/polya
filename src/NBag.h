#pragma once

#ifdef DENSEHASH
# include <sparsehash/dense_hash_map>
# define NBAG_MAP google::dense_hash_map<N,N>
#else
# include <unordered_map>
# define NBAG_MAP std::unordered_map<N,N>
#endif

#include "util.h"

class NBag {
    public:
    NBAG_MAP m;
    N t; // number of non-zero elements
    N c; // sum of all elements

    NBag() : t(0), c(0) {
#ifdef DENSEHASH
        m.set_deleted_key(N_INFTY);
        m.set_empty_key(0);
#endif
    }

    bool inc(N x) {
        assert(x == 0 || m[x] > 0);
        m[x+1] += 1; c += 1;
        if(x > 0) {
            m[x] -= 1;
            if(m[x] == 0) m.erase(x);
            return false;
        } else { // new non-zero element
            t += 1;
            return true;
        }
    }

    bool dec(N x) {
        assert(x > 0 && m[x] > 0 && t > 0);
        m[x] -= 1; c -= 1;
        if(m[x] == 0) m.erase(x);
        if(x > 1) {
            m[x-1] += 1;
            return false;
        } else { // now zero
            t -= 1;
            return true;
        }
    }

#ifndef NDEBUG
    void check() const {
        N card = 0, sum = 0;
        FOR_PAIR(x,n, m) { sum += n * x; if(x != 0) card += n; }
        ASSERT(,t, <= ,c,); ASSERT(,t, == ,card,); ASSERT(,c, == ,sum,);
    }
#endif
};
