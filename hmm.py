import numpy as np

from logarithmetic import *

class HMM:
    def __init__(self, trans, emit, M, S, states=[]):
        self.trans = trans   # transition function
        self.emit = emit     # emission function
        self.M = M           # context length
        self.S = S           # size of (hidden) state space
        self.states = states # hiden state sequence
    
    def T(self, t):
        u = tuple(self.states[max(0,t-self.M):t])
        return self.trans[u]
    def E(self, t):
        return self.emit[self.states[t]]
    
    def resample(self, text, update=True):
        joint = LogR(1)
        for t,w in enumerate(text):
            # index of next state independent of the current position t
            T = min(t + self.M + 1, len(self.states))
            
            if update: # forget old state, and states depending on it
                self.E(t).forget(w)
                for i in range(t,T): self.T(i).forget(self.states[i])
            
            # posterior distribution over states
            post = np.zeros(self.S)
            for s in range(self.S):
                self.states[t] = s
                p = self.E(t)(w)
                for i in range(t,T): p *= self.T(i)(self.states[i])
                post[s] = p
            post /= np.sum(post)
            
            # sample new state from posterior
            self.states[t] = np.random.choice(self.S, p=post)
            joint *= post[self.states[t]]
            
            if update: # update model
                self.E(t).observe(w)
                for i in range(t,T): self.T(i).observe(self.states[i])
        
        return joint
    
    def standardise(self, gold):
        # frequency of gold standard tags per cluster
        freq = [{'<NULL>':0} for s in range(self.S)]
        for s,g in zip(self.states, gold):
            freq[s][g] = freq[s].get(g,0) + 1
        
        # preferred (gold) tag for each cluster
        pref = [max(f.keys(), key=lambda g: f[g]) for f in freq]
        std = [pref[s] for s in self.states]
        
        # many-to-one (M-1) performance measure
        M1 = float(sum(t == g for t,g in zip(std,gold))) / len(std)
        
        return std, M1
    
    def M1(self, gold):
        return self.standardise(gold)[1]
