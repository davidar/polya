#pragma once

#include <string>

#include "util.h"

class Trie {
    R weight, leaf;
    std::map <char,Trie> child;
    
    public:
    Trie() : weight(0), leaf(0) {}
    
    void clear() {
        weight = leaf = 0;
        FOR_VAL(c,child) c.clear();
    }
    
    void insert(std::string s, R w) { insert(s.begin(), s.size(), w); }
    template<typename I>
    void insert(I s, N len, R w) {
        weight += w;
        if(len == 0) leaf += w;
        else child[*s].insert(s+1, len-1, w);
    }
    
    const Trie *lookup(std::string s) const DO(lookup(s.c_str(), s.size()))
    template<typename I>
    const Trie *lookup(I s, N len) const {
        if(len == 0) return this;
        IF_FIND(*s,c,child) return c.lookup(s+1, len-1);
        return NULL;
    }
    
    R predict(char x = 0) const {
        if(!x) return leaf / weight;
        IF_FIND(x,c,child) return c.weight / weight;
        return 0;
    }
};
