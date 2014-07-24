import sys

import pyximport; pyximport.install()
from polya import Polya, PolyaHyper, Uniform, Mixture, Odds, OddsTerm

from util import *
from logarithmetic import *
from corpus import *

def mask(u):
    return tuple(map(lambda w: w is not None, u))

def base(u, G, gamma):
    Hs = []; terms = []
    for i,x in enumerate(u):
        if x is not None:
            v = list(u); v[i] = None; v = tuple(v)
            Hs.append(G[v])
            terms.append(gamma[v])
    if   len(Hs) == 0: return G[0]
    elif len(Hs) == 1: return Hs[0]
    else: return Mixture(Odds(*terms), *Hs)

def sparse_hpylm():
    hyper = DynDict(lambda u: PolyaHyper())
    gamma = DynDict(lambda u: OddsTerm())
    G = DynDict(lambda u: Polya(hyper[mask(u)], base(u, G, gamma)))
    return G

def main(ngram=3, test_size=100000):
    M = int(ngram) - 1; test_size = int(test_size)
    vocab, train, test = read_data(test_size)
    G = sparse_hpylm(); G[0] = Uniform(len(vocab))
    for u,w in contexts(train, M, 'initialising...'): G[u] += w
    run(lambda: LogR.product(G[u](w) for u,w in contexts(test,M)), len(test))
if __name__ == '__main__': main(*sys.argv[1:])
