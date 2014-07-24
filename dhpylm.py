#!/usr/bin/env python
import sys

import pyximport; pyximport.install()
import polya
from polya import Polya, PolyaHyper, Uniform, Mixture

from util import *
from logarithmetic import *
from corpus import *
from hpylm import hpylm

def domain_model(G0, depth, S, L):
    # need to separate this function out of the loop so that
    # the closure in G has the correct environment
    h = [PolyaHyper() for i in range(depth)]
    G = DynDict(lambda u:
        Polya(h[len(u)], Mixture(S[len(u)], G[u[1:]], L[u])))
    G[()] = Polya(h[0], Mixture(S[0], G0, L[()]))
    return G

def dhpylm(G0, depth, ndom):
    L = hpylm(G0, depth) # latent language model
    switch_hypers = [PolyaHyper() for i in range(depth)]
    Gs = [] # domain-specific contextual distributions
    for i in range(ndom):
        switch = [Polya(switch_hypers[j], polya.coin) for j in range(depth)]
        Gs.append(domain_model(G0, depth, switch, L))
    return Gs

def main(ngram, ftest0, *ftrains):
    M = int(ngram) - 1
    vocab = DynEnum()
    test0 = read_corpus(vocab, ftest0)
    trains = [read_corpus(vocab, f) for f in ftrains]
    G0 = Uniform(len(vocab))
    G = dhpylm(G0, M+1, len(trains))
    for i,train in enumerate(trains):
        msg = 'initialising domain ' + str(i) + '...'
        for u,w in contexts(train, M, msg): G[i][u] += w
    run(lambda: LogR.product(G[0][u](w) for u,w in contexts(test0,M)),
        len(test0))
if __name__ == '__main__': main(*sys.argv[1:])
