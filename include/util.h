#pragma once

#undef NDEBUG

#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <assert.h>

#include <string>

typedef double R; // real
typedef unsigned int N; // natural
typedef unsigned int X; // datapoint

const N N_INFTY =  UINT_MAX;

inline R cputime() {
    static const clock_t start = clock();
    return (R) (clock() - start) / CLOCKS_PER_SEC;
}

#define SELF (*this)

#define LOG(fmt,...) \
    printf("[%7.1fs] " fmt "\n", cputime(), ##__VA_ARGS__) && fflush(stdout)

#ifndef NDEBUG
# define DBG(fmt,...) \
    fprintf(stderr, "%s:%u " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
# define DBG(...)
#endif

#define STR(x) (std::to_string(x).c_str())

// overload macros on number of args
// http://stackoverflow.com/q/11761703
#define OVERLOAD(F,...) \
    _OVERLOAD(__VA_ARGS__, \
        F##9,F##8,F##7,F##6,F##5,F##4,F##3,F##2,F##1)(__VA_ARGS__)
# define _OVERLOAD(_1,_2,_3,_4,_5,_6,_7,_8,_9,F,...) F

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
    LET(auto &k __attribute__((unused)) = kv.first) \
    LET(auto &v __attribute__((unused)) = kv.second)
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
# define DO5(a,b,c,d,x) {a;b;c;d; return (x);}
# define DO4(a,b,c,x) DO5(a,b,c,,x)
# define DO3(a,b,x) DO4(a,b,,x)
# define DO2(a,x) DO3(a,,x)
# define DO1(x) DO2(,x)

// ASSERT(... ,u, ... ,v, ...) prints the value/s between commas on failure
#define ASSERT(...) OVERLOAD(ASSERT,__VA_ARGS__)
# define ASSERT3(l,v,r) assert((l v r) || _ASSERT3(l,v,r))
#  define _ASSERT3(l,v,r) \
    !DBG("Assertion `%s (%s = %s) %s' failed.", #l, #v, STR(v), #r)
# define ASSERT5(l,u,m,v,r) assert((l u m v r) || _ASSERT5(l,u,m,v,r))
#  define _ASSERT5(l,u,m,v,r) \
    !DBG("Assertion `%s (%s = %s) %s (%s = %s) %s' failed.", \
            #l, #u, STR(u), #m, #v, STR(v), #r)

// IF_FIND(k,v,m) {...} = if(m.count(k)) {auto &v = m.at(k); ...}
#define IF_FIND(k,v,m) _IF_FIND(k,v,m,UNIQ)
# define _IF_FIND(k,v,m,i) LET(auto i = m.find(k)) \
    if(i != m.end()) LET(auto &v = i->second)

// positive/negative parts x^+,x^-
inline R pos(R x) DO((x > 0) ? x : 0)
inline R neg(R x) DO(pos(-x))
