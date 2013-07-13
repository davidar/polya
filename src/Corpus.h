#pragma once

#include <string>
#include <iostream>

#include <boost/bimap.hpp>

#include "util.h"

class Corpus : public std::vector<X> {
    typedef boost::bimap<X,std::string> dict_type;
    typedef dict_type::value_type dict_entry;
    dict_type dict;

    public:
    Corpus(std::istream &stream) {
        std::string s; while(stream >> s) push(s);
    }

    Corpus() : Corpus(std::cin) {
        LOG("read %u words (%u unique)", (N) size(), vocab_size());
    }

    N vocab_size() DO(dict.size())

    void push(std::string s) {
        if(!dict.right.count(s))
            dict.insert(dict_entry(vocab_size() + 1, s));
        push_back(dict.right.at(s));
    }

    std::string lookup(X x) const DO(dict.left.at(x))
};
