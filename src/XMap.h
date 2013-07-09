#pragma once

#include <stdexcept>

#include <sparsehash/sparse_hash_map>
using google::sparse_hash_map;

#include "util.h"

template <class T>
class XMap : public sparse_hash_map<X,T> {
    typedef sparse_hash_map<X,T> super;

    public:
    XMap() {
        super::set_deleted_key(X_INVALID);
    }

    const T &at(X x) const {
        if(!super::count(x)) throw std::out_of_range("XMap::at");
        return super::find(x)->second;
    }
};
