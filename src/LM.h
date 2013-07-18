#pragma once

#include "util.h"
#include "Corpus.h"
#include "DUnif.h"
#include "LogR.h"

class LM {
    public:
    Corpus text;
    DUnif uvocab;

    LM() : text(), uvocab(text.vocab_size()) {}

    static R pplx(LogR p, N n) DO((R) pow(p, -1./n))

    virtual void train(N train_size) = 0;
    virtual void resamp() = 0;
    virtual R pred(Corpus::iterator i) = 0;

    R run(N test_size, N n_iter, N n_burnin) {
        LOG("%u test words, (%u - %u) iterations",
                test_size, n_iter, n_burnin);
        N train_size = text.size() - test_size; train(train_size);
        LOG("finished training (%u words)", train_size);
        R start_time = cputime();

        LogR p_tot; FOR(i,n_iter) {
            LOG("*** ITER %u/%u", i+1, n_iter);

            resamp();
            LogR p(1); FOR_ITER(i, text, train_size, test_size) p *= pred(i);
            if(i >= n_burnin) p_tot += p;

            LOG("perplexity = %g", pplx(p, test_size));
            R eta = (n_iter - (i+1)) * (cputime() - start_time) / (i+1);
            LOG("ETA %um", (N) ceil(eta / 60));
        }

        LOG("finished testing");
        return pplx(p_tot / (n_iter - n_burnin), test_size);
    }
};
