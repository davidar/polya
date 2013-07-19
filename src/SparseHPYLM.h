#pragma once

#include "util.h"
#include "LM.h"
#include "SparseCtxt.h"

template <N L>
class SparseHPYLM : public LM {
    std::vector<CRP::Hyper> hr;
    SparseCtxt<L> ct;

    CRP &context(Corpus::iterator i) {
        auto start = max(text.begin(), i - L);
        return ct[Ctxt<L>(start,i)];
    }

    public:
    SparseHPYLM() : hr(1<<L), ct(hr,uvocab) {}
    void train(N train_size)
        { FOR_ITER(i, text, train_size) context(i) += *i; }
    void resamp() { ct.resamp(); for(auto &h : hr) h.resamp(); }
    R pred(Corpus::iterator i) DO(context(i)(*i))
};
