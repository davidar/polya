#pragma once

#include "util.h"

class Exch {
    public:
    virtual ~Exch() {}
    virtual R operator()(X x) const = 0; // pmf
    virtual X operator()() const = 0; // sample
    virtual Exch &operator+=(X x) = 0; // add observation
    virtual Exch &operator-=(X x) = 0; // remove observation
    virtual void resamp() = 0; // resample internal state

    virtual void ser(FILE *f) const = 0;
};
