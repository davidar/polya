import sys
import random
import numpy as np

from eta import ETA

import pyximport; pyximport.install()
import polya
from polya import Polya, PolyaHyper, Uniform

from util import *
from logarithmetic import *

from hmm import HMM
from hpylm import hpylm

class PYPHMM(HMM):
    def __init__(self, nwords, S, M=2):
        self.M = M; self.S = S
        self.trans = hpylm(Uniform(self.S), M+1)
        Eh = PolyaHyper('HMM-E'); E0 = Uniform(nwords)
        self.emit = [Polya(Eh, E0) for i in range(self.S)]
    
    def train(self, text):
        self.states = [random.randrange(self.S) for w in text]
        for t,w in enumerate(text):
            self.T(t).observe(self.states[t])
            self.E(t).observe(w)

def main(n_iter=500):
    random.seed(0); np.random.seed(0)
    text_vocab = DynEnum(); tag_vocab = DynEnum()
    text = []; gold_tags = []
    for line in sys.stdin:
        word,tag = line.split()
        text.append(text_vocab(word))
        gold_tags.append(tag_vocab(tag))
    
    hmm = PYPHMM(len(text_vocab), len(tag_vocab))
    hmm.train(text)
    
    eta = ETA(n_iter); eta.print_status(0, extra='starting...')
    for i in range(n_iter):
        polya.resample()
        post = hmm.resample(text)
        eta.print_status(i+1, extra="POST %.2fb | M-1 %.2f%%" \
            % (-log(post,2) / len(text), hmm.M1(gold_tags) * 100))
    eta.done()
    print>>sys.stderr, 'M-1:', hmm.M1(gold_tags) * 100
if __name__ == '__main__': main()
