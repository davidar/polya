#pragma once

#include <utility>

#include "util.h"
#include "rand.h"
#include "LogR.h"

#define SLICESAMP_MAXSTEPS 32
#define SLICESAMP_MAXSHRINK 1024

// see RM Neal (2003) "Slice Sampling" doi:10.1214/aos/1056562461
// also http://web.science.mq.edu.au/~mjohnson/Software.htm
class SliceSamp {
    typedef std::pair<R,R> interval_t;

    R w; // estimate of typical slice size

    interval_t step(R x0, R y) const { // Neal Fig3
        const N m = SLICESAMP_MAXSTEPS;
        R l = x0 - w * randu(), r = l + w;
        N j = rand() % m, k = (m-1) - j;
        for(; j > 0 && y < log(f(l)); j--) l -= w;
        for(; k > 0 && y < log(f(r)); k--) r += w;
        return std::make_pair(l,r);
    }

    R shrink(R x0, R y, interval_t I) const { // Neal Fig5
        R l = I.first, r = I.second;
        FOR(SLICESAMP_MAXSHRINK) {
            R x1 = randu(l,r);
            if(y < log(f(x1))) return x1;
            if(x1 < x0) l = x1;
            else        r = x1;
        }
        WARN("SliceSamp::shrink(%g, %g, (%g,%g)) failed",
            x0, y, I.first, I.second);
        return x0;
    }

    public:
    SliceSamp(R width = 1) : w(width) {}

    virtual void init() {};
    virtual LogR f(R x) const = 0;

    R operator()(R x0) { // Neal Sec4
        init();
        R y = log(f(x0)) - rand_exp();
        interval_t I = step(x0, y);
        R x1 = shrink(x0, y, I);
        w = (w + 1.5 * fabs(x1 - x0)) / 2;
        return x1;
    }
};
