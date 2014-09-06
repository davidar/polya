#pragma once

#include <string>

#include "util.h"

class Trie {
    R weight, leaf;
    Trie *child[0x100];
    
    public:
    Trie() : weight(0), leaf(0) {
        memset(child, 0, sizeof child);
    }
    
    void clear() {
        weight = leaf = 0;
        FOR(c,0x100) if(child[c]) child[c]->clear();
    }
    
    void insert(std::string s, R w) { insert(s.c_str(), w); }
    void insert(const char *s, R w) {
        weight += w;
        char c = s[0];
        if(c == '\0') {
            leaf += w;
        } else {
            if(child[c] == NULL) child[c] = new Trie();
            child[c]->insert(s+1, w);
        }
    }
    
    const Trie *lookup(std::string s) const DO(lookup(s.c_str(), s.size()))
    template<typename I>
    const Trie *lookup(I s, N len) const {
        if(len == 0) return this;
        if(child[*s] == NULL) return NULL;
        return child[*s]->lookup(s+1, len-1);
    }
    
    R predict(char c = '\0') const {
        if(c == '\0') return leaf / weight;
        if(child[c] == NULL) return 0;
        return child[c]->weight / weight;
    }
};
