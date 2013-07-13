#pragma once

#include "util.h"

class Exch {
    public:
    virtual ~Exch() {}
    virtual R pred(X x) const = 0;
    virtual X samp() const = 0;
    virtual void add(X x) = 0;
    virtual void del(X x) = 0;
    virtual void resamp() = 0;

    virtual void ser(FILE *f) const = 0;
};
