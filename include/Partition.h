#pragma once

#include <unordered_map>

#include "util.h"

class Partition {
    public:
    std::unordered_map<N,N> m; // multiplicities
    N t; // number of non-zero elements
    N c; // sum of all elements

    Partition() : t(0), c(0) {}

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
};
