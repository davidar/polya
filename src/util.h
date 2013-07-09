#pragma once

#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <assert.h>

typedef double R; // real
typedef unsigned int N; // natural
typedef unsigned int X; // datapoint

const N N_INFTY =  UINT_MAX;
const X X_INVALID = UINT_MAX;
const X X_NULL = 0;

inline R cputime() {
    static const clock_t start = clock();
    return (R) (clock() - start) / CLOCKS_PER_SEC;
}

#define SELF (*this)

#define LOG(fmt,...) { \
    printf("[%7.1fs] " fmt, cputime(), ##__VA_ARGS__); \
    fflush(stdout); \
}

// overload macros on number of args
// http://stackoverflow.com/q/11761703
#define OVERLOAD(F,...) \
    _OVERLOAD(__VA_ARGS__,F##5,F##4,F##3,F##2,F##1)(__VA_ARGS__)
# define _OVERLOAD(_1,_2,_3,_4,_5,F,...) F

// messy indirection for creating unique variable names
// http://stackoverflow.com/q/1597007
#define UNIQ TOKPASTE(_,__COUNTER__)
# define TOKPASTE(x,y) _TOKPASTE(x,y)
#  define _TOKPASTE(x,y) x##y

// FOR(i,a,b) = for i in [a,b)
// FOR(i,n)   = for i in [0,n)
// FOR(n)     = repeat n times
#define FOR(...) OVERLOAD(FOR,__VA_ARGS__)
# define FOR3(i,a,b) for(N i = a; i < b; i++)
# define FOR2(i,n) FOR3(i,0,n)
# define FOR1(n) FOR2(UNIQ,n)

// LET(e) {...} = {e; ...}
#define LET(e) _LET(e,UNIQ)
# define _LET(e,b) for(bool b=true;b;) for(e;b;b=false)

// FOR_PAIR(k,v,m) = for k,v in m
#define FOR_PAIR(k,v,m) _FOR_PAIR(k,v,m,UNIQ)
# define _FOR_PAIR(k,v,m,kv) for(auto &kv : m) \
    LET(auto &k = kv.first) LET(auto &v = kv.second)
#define FOR_KEY(k,m) FOR_PAIR(k,UNIQ,m)
#define FOR_VAL(v,m) FOR_PAIR(UNIQ,v,m)

// FOR_ITER(i,v,c,n) = for *i in v[c:c+n)
// FOR_ITER(i,v,n)   = for *i in v[0:n)
// FOR_ITER(i,v)     = for *i in v
#define FOR_ITER(...) OVERLOAD(FOR_ITER,__VA_ARGS__)
# define FOR_ITER4(i,v,c,n) _FOR_ITER4(i,v,c,n,UNIQ)
#  define _FOR_ITER4(i,v,c,n,j) LET(N j = 0) \
    for(auto i = v.begin() + c; i != v.end() && j < n; i++, j++)
# define FOR_ITER3(i,v,n) FOR_ITER4(i,v,0,n)
# define FOR_ITER2(i,v) FOR_ITER3(i,v,N_INFTY)

// DO(...,x) = {...; return x;}
#define DO(...) OVERLOAD(DO,__VA_ARGS__)
# define DO2(e,x) {e; return (x);}
# define DO1(x) DO2(,x)

// positive/negative parts x^+,x^-
inline R pos(R x) DO((x > 0) ? x : 0)
inline R neg(R x) DO(pos(-x))
