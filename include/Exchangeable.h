#pragma once

#include <string>

#include "util.h"

class Exchangeable {
    public:
    virtual ~Exchangeable() {}

    virtual R predict(X x) const = 0; // pmf
    virtual X sample() const = 0; // sample
    virtual Exchangeable &observe(X x) = 0; // add observation
    virtual Exchangeable &forget(X x) = 0; // remove observation
    virtual void resample() = 0; // resample internal state

    R operator()(X x) const DO(predict(x))
    virtual X operator()() const DO(sample())
    virtual Exchangeable &operator+=(X x) DO(observe(x))
    virtual Exchangeable &operator-=(X x) DO(forget(x))

    virtual std::string toString() const = 0;
};
