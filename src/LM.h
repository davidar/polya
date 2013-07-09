#pragma once

#include "util.h"
#include "Corpus.h"
#include "LogR.h"

class LM {
    public:
    Corpus text;

    static R pplx(LogR p, N n) DO((R) pow(p, -1./n))

    virtual void train(N train_size) = 0;
    virtual void resamp() = 0;
    virtual R pred(Corpus::iterator i) = 0;

    R run(N test_size, N n_iter, N n_burnin) {
        LOG("%d test words, (%d - %d) iterations\n",
                test_size, n_iter, n_burnin);
        N train_size = text.size() - test_size; train(train_size);
        LOG("finished training (%d words)\n\n", train_size);
        LogR p_tot; FOR(i,n_iter) {
            LOG("iteration %d/%d\n", i+1, n_iter);
            resamp();
            LogR p(1); FOR_ITER(i, text, train_size, test_size) p *= pred(i);
            LOG("perplexity = %g\n\n", pplx(p, test_size));
            if(i >= n_burnin) p_tot += p;
        }
        LOG("finished testing\n");
        return pplx(p_tot / (n_iter - n_burnin), test_size);
    }
};
