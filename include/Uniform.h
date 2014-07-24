#pragma once

#include <sstream>
#include <stdlib.h>

#include "util.h"
#include "Exchangeable.h"

class Uniform : public Exchangeable {
    public:
    const N n;

    Uniform(N num) : n(num) {}
    R predict(X x) const DO((0 <= x && x < n) ? (1./n) : 0)
    X sample() const DO(rand() % n)
    Exchangeable &observe(X x) DO(SELF)
    Exchangeable &forget(X x) DO(SELF)
    void resample() {}

    std::string toString() const {
        std::stringstream ss;
        ss << "Uniform(" << n << ")\n";
        return ss.str();
    }
};
