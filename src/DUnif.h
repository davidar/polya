#pragma once

#include <stdlib.h>

#include "util.h"
#include "Exch.h"

class DUnif : public Exch {
    public:
    const N n;

    DUnif(N num) : n(num) {}
    R operator()(X x) const DO((x != X_NULL && x <= (X) n) ? (1./n) : 0)
    X operator()() const DO((X)(1 + rand() % n))
    Exch &operator+=(X x) DO(SELF)
    Exch &operator-=(X x) DO(SELF)
    void resamp() {}

    void ser(FILE *f) const { fprintf(f, "DUnif(%u)\n", n); }
};
