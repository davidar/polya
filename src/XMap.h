#pragma once

#ifdef SPARSEHASH
# include <stdexcept>
# include <sparsehash/sparse_hash_map>
# define XMAP_SUPER google::sparse_hash_map<X,T>
#else
# include <unordered_map>
# define XMAP_SUPER std::unordered_map<X,T>
#endif

#include "util.h"

template <class T>
class XMap : public XMAP_SUPER {
    typedef XMAP_SUPER super;

    public:
#ifdef SPARSEHASH
    XMap() {
        super::set_deleted_key(X_INVALID);
    }

    const T &at(X x) const {
        if(!super::count(x)) throw std::out_of_range("XMap::at");
        return super::find(x)->second;
    }
#endif
};
