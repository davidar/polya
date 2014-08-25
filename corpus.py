import sys

from eta import ETA # https://pypi.python.org/pypi/eta

import pyximport; pyximport.install()
import polya

from util import *
from logarithmetic import *

def contexts(text, M, verbose=None):
    if verbose: eta = ETA(len(text))
    for i in range(len(text)):
        yield tuple(text[max(0,i-M):i]), text[i]
        if verbose: eta.print_status(i, extra=verbose)
    if verbose: eta.done()

def read_corpus(vocab, f=sys.stdin):
    if type(f) is str: f = open(f, 'r')
    text = []
    for line in f:
        text += map(vocab, line.split())
    return text

def read_data(test_size, corpus=sys.stdin):
    vocab = DynEnum()
    text = read_corpus(vocab, corpus)
    train_size = len(text) - test_size
    assert train_size > 0
    train = text[:train_size]; test = text[train_size:]
    return vocab, train, test

def run(predict, test_size, n_iter=100, n_burnin=10, resample=None):
    p_tot = LogR(0)
    eta = ETA(n_iter); eta.print_status(0, extra='starting...')
    for i in range(n_iter):
        print polya.timestamp(), "iteration %u/%u" % (i+1, n_iter)
        polya.resample()
        if resample: resample()
        
        p = predict()
        pplx = float(p ** (-1./test_size))
        print polya.timestamp(), 'perplexity =', pplx
        if i < n_burnin:
            eta.print_status(i+1, extra="burning in (%.1f)..." % pplx)
        else:
            p_tot += p
            pplx = float((p_tot / (i+1 - n_burnin)) ** (-1./test_size))
            eta.print_status(i+1, extra="perplexity %.1f" % pplx)
    eta.done()
    p_avg = p_tot / (n_iter - n_burnin)
    pplx = float(p_avg ** (-1./test_size))
    print '---\nfinal perplexity =', pplx
    print>>sys.stderr, 'Perplexity:', pplx
    return p_avg
