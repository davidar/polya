import sys
import random
import numpy as np

from eta import ETA

import pyximport; pyximport.install()
import polya
from polya import Polya, PolyaHyper, Uniform, HMM

from util import *
from logarithmetic import *

from hpylm import hpylm

def pyphmm(text, nwords, S, M=2):
    trans = hpylm(Uniform(S), M+1)
    Eh = PolyaHyper('HMM-E'); E0 = Uniform(nwords)
    emit = [Polya(Eh, E0) for i in range(S)]
    return HMM(text, trans, emit, M, S)

def main(n_iter=500):
    random.seed(0); np.random.seed(0)
    text_vocab = DynEnum(); tag_vocab = DynEnum()
    text = []; gold_tags = []
    for line in sys.stdin:
        word,tag = line.split()
        text.append(text_vocab(word))
        gold_tags.append(tag_vocab(tag))
    
    hmm = pyphmm(text, len(text_vocab), len(tag_vocab))
    hmm.train()
    
    eta = ETA(n_iter); eta.print_status(0, extra='starting...')
    for i in range(n_iter):
        print 'iteration', i
        polya.resample()
        post = hmm.resample()
        status = "POST %.2fb | M-1 %.2f%%" \
            % (-log(post,2) / len(text), hmm.M1(gold_tags) * 100)
        print 'status:', status
        eta.print_status(i+1, extra=status)
    eta.done()
    print>>sys.stderr, 'M-1:', hmm.M1(gold_tags) * 100
if __name__ == '__main__': main()
