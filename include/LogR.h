#pragma once

#include <math.h>

#include "util.h"

struct LogR {
    R ln;

    LogR(R x) : ln(log(x)) {}
    LogR() : LogR(0) {}
    explicit operator R() const DO(exp(ln))

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
inline LogR lpow(R a, R x) DO(pow(LogR(a), x))

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
inline LogR LogR::operator+=(LogR b) DO(SELF = SELF + b)
inline LogR LogR::operator-=(LogR b) DO(SELF = SELF - b)


inline LogR lgam(R x) DO(LogR r, r.ln = lgamma(x), r)
inline LogR lexp(R x) DO(LogR r, r.ln = x, r)

// rising factorial / k-Pochhammer symbol
inline LogR lpoch(R x, N n)
    DO((n > 0) ? lgam(x + n) / lgam(x) : LogR(1))
inline LogR lpoch(R x, N n, R k)
    DO((k > 0) ? lpow(k,n) * lpoch(x/k,n) : lpow(x,n))
