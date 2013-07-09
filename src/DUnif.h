#pragma once

#include <stdlib.h>

#include "util.h"
#include "Exch.h"

class DUnif : public Exch {
    public:
    const N n;

    DUnif(N num) : n(num) {}
    R pred(X x) const DO((0 < x && x <= n) ? (1./n) : 0)
    X samp() const DO(1 + rand() % n)
    void add(X x) {}
    void del(X x) {}
    void resamp() {}
};
