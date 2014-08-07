#!/usr/bin/env python
import sys

import pyximport; pyximport.install()
from polya import Polya, PolyaHyper, Uniform

from util import *
from logarithmetic import *
from corpus import *

def hpylm(G0, depth):
    h = [PolyaHyper("HPYLM(%d)" % i) for i in range(depth)]
    G = DynDict(lambda u: Polya(h[len(u)], G[u[1:]]))
    G[()] = Polya(h[0], G0) # context-free distribution
    return G

def main(ngram=3, test_size=100000):
    M = int(ngram) - 1; test_size = int(test_size)
    vocab, train, test = read_data(test_size)
    G0 = Uniform(len(vocab)) # global base distribution
    G = hpylm(G0, M+1)
    for u,w in contexts(train, M, 'initialising...'): G[u] += w
    run(lambda: LogR.product(G[u](w) for u,w in contexts(test,M)), len(test))
if __name__ == '__main__': main(*sys.argv[1:])
