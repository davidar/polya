#pragma once

#include "util.h"
#include "CRP.h"
#include "DUnif.h"
#include "CtxtTree.h"
#include "Corpus.h"
#include "LM.h"

class HPYLM : public LM {
    const N ngram;
    std::vector<CRP::Hyper> hs;
    DUnif base;
    CtxtTree ct;

    Corpus::iterator ctxt_start(Corpus::iterator i)
        DO(max(text.begin(), i - ngram + 1))

    public:
    HPYLM(N n) : ngram(n), hs(n), base(text.vocab_size()), ct(base,hs) {}

    void train(N train_size) {
        FOR_ITER(i, text, train_size)
            ct.context(ctxt_start(i),i).add(*i);
    }

    void resamp() { ct.resamp(); for(auto &h : hs) h.resamp(); }
    R pred(Corpus::iterator i) DO(ct.context_at(ctxt_start(i),i).pred(*i))
};
