#pragma once

#include <map>

#include "util.h"

template <typename K, typename V>
class DynDict {
    public:
    typedef V (*F)(K, void*);
    
    private:
    F f;
    std::map<K,V> m;
    void *user_data;
    
    public:
    DynDict(F f, void *user_data) : f(f), user_data(user_data) {}
    
    V &operator[](const K &k) {
        IF_FIND(k,v, m) return v;
        return m[k] = f(k, user_data);
    }
};
