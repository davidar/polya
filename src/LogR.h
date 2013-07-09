#pragma once

#include <math.h>

#include "util.h"

struct LogR {
    R ln;

    LogR(R x) : ln(log(x)) {}
    LogR() : LogR(0) {}
    explicit operator R() DO(exp(ln))

    LogR operator*=(R x) DO(ln += log(x), SELF)
    LogR operator/=(R x) DO(ln -= log(x), SELF)
    LogR operator*=(LogR b) DO(ln += b.ln, SELF)
    LogR operator/=(LogR b) DO(ln -= b.ln, SELF)
    LogR operator+=(LogR b);
    LogR operator-=(LogR b);
};

inline R log (LogR a) DO(a.ln)
inline R log2(LogR a) DO(a.ln / log(2))

inline LogR pow(LogR a, R x) DO(a.ln *= x, a)

inline LogR operator*(LogR a, R x) DO(a *= x)
inline LogR operator/(LogR a, R x) DO(a /= x)
inline LogR operator*(LogR a, LogR b) DO(a *= b)
inline LogR operator/(LogR a, LogR b) DO(a /= b)

inline bool operator==(LogR a, LogR b) DO(a.ln == b.ln)
inline bool operator!=(LogR a, LogR b) DO(!(a == b))
inline bool operator< (LogR a, LogR b) DO(a.ln < b.ln)
inline bool operator> (LogR a, LogR b) DO(b < a)
inline bool operator<=(LogR a, LogR b) DO(!(a > b))
inline bool operator>=(LogR a, LogR b) DO(!(a < b))

inline LogR operator+(LogR a, LogR b) DO((a >= b) ? a *= 1+(R)(b/a) : b+a)
inline LogR operator-(LogR a, LogR b) DO(assert(a >= b), a *= 1-(R)(b/a))
LogR LogR::operator+=(LogR b) DO(SELF = SELF + b)
LogR LogR::operator-=(LogR b) DO(SELF = SELF - b)
