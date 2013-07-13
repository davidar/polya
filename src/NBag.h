#pragma once

#include <sparsehash/dense_hash_map>
using google::dense_hash_map;

#include "util.h"

class NBag : public dense_hash_map<N,N> {
    typedef dense_hash_map<N,N> super;

    public:
    N t; // number of non-zero elements
    N c; // sum of all elements

    NBag() : t(0), c(0) {
        super::set_deleted_key(N_INFTY);
        super::set_empty_key(0);
    }

    bool inc(N x) {
        assert(x == 0 || SELF[x] > 0);
        SELF[x+1] += 1; c += 1;
        if(x > 0) {
            SELF[x] -= 1;
            if(SELF[x] == 0) super::erase(x);
            return false;
        } else { // new non-zero element
            t += 1;
            return true;
        }
    }

    bool dec(N x) {
        assert(x > 0 && SELF[x] > 0 && t > 0);
        SELF[x] -= 1; c -= 1;
        if(x > 1) {
            SELF[x-1] += 1;
            return false;
        } else { // now zero
            t -= 1;
            return true;
        }
    }

#ifndef NDEBUG
    void check() const {
        N card = 0, sum = 0;
        FOR_PAIR(x,n, SELF) { sum += n * x; if(x != 0) card += n; }
        ASSERT(,t, <= ,c,); ASSERT(,t, == ,card,); ASSERT(,c, == ,sum,);
    }
#endif
};
