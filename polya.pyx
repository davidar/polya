from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.vector cimport vector
from cpython.exc cimport PyErr_CheckSignals

cimport _polya
from _polya cimport R, N, X

from logarithmetic import LogR

_polya.init()

def cputime():
    return _polya.cputime()

def timestamp():
    return "[%7.1fs]" % _polya.cputime()

nodes = []
def resample():
    for node in nodes:
        node.resample()
        PyErr_CheckSignals() # check if loop aborted

cdef class Exchangeable:
    def __cinit__(self):
        self.thisptr = NULL
        nodes.append(self)
    
    def predict(self, X x):
        return self.thisptr.predict(x)
    def sample(self):
        return self.thisptr.sample()
    def observe(self, X x):
        self.thisptr.observe(x)
        return self
    def forget(self, X x):
        self.thisptr.forget(x)
        return self
    def resample(self):
        self.thisptr.resample()
    
    def __call__(self, x=None):
        if x is None:
            return self.sample()
        else:
            return self.predict(x)
    def __iadd__(self, x):
        return self.observe(x)
    def __isub__(self, x):
        return self.forget(x)
    def __str__(self):
        return self.thisptr.toString()

cdef class Uniform(Exchangeable):
    def __cinit__(self, N n):
        self.thisptr = <_polya.Exchangeable*> new _polya.Uniform(n)
    property n:
        def __get__(self): return (<_polya.Uniform*> self.thisptr).n

coin = Uniform(2)

cdef class PolyaHyper:
    cdef _polya.PolyaHyper *thisptr
    def __cinit__(self, name=''):
        self.thisptr = new _polya.PolyaHyper(name)
        nodes.append(self)
    def resample(self):
        self.thisptr.resample()

cdef class Polya(Exchangeable):
    def __cinit__(self, PolyaHyper hyper, Exchangeable base):
        self.thisptr = <_polya.Exchangeable*> new _polya.Polya(
            deref(hyper.thisptr), deref(base.thisptr))
    property c:
        def __get__(self): return (<_polya.Polya*> self.thisptr).c
    property t:
        def __get__(self): return (<_polya.Polya*> self.thisptr).t

cdef class Mixture(Exchangeable):
    def __cinit__(self, Exchangeable pi, *components):
        cdef vector[_polya.Exchangeable*] cs
        for c in components: cs.push_back((<Exchangeable> c).thisptr)
        self.thisptr = <_polya.Exchangeable*> new _polya.Mixture(
            deref(pi.thisptr), cs)

cdef class OddsTerm:
    cdef _polya.OddsTerm *thisptr
    def __cinit__(self):
        self.thisptr = new _polya.OddsTerm()
        nodes.append(self)
    def resample(self):
        self.thisptr.resample()

cdef class Odds(Exchangeable):
    def __cinit__(self, *terms):
        cdef vector[_polya.OddsTerm*] ts
        for t in terms: ts.push_back((<OddsTerm> t).thisptr)
        self.thisptr = <_polya.Exchangeable*> new _polya.Odds(ts)

cdef _polya.Exchangeable *hmm_cb(vector[unsigned int] context, void *trans):
    u = tuple(context)
    res = (<object>trans)[u]
    return (<Exchangeable> res).thisptr

cdef class HMM:
    cdef _polya.HMM *thisptr
    def __cinit__(self, text, trans, emit, N M, N K):
        cdef vector[_polya.Exchangeable*] es
        for e in emit: es.push_back((<Exchangeable> e).thisptr)
        self.thisptr = new _polya.HMM(M, K, text, es, hmm_cb, <void*> trans)
    
    def train(self):
        self.thisptr.train()
    
    def resample(self):
        return LogR.exp(self.thisptr.resample())
    
    def standardise(self, gold):
        # frequency of gold standard tags per cluster
        freq = [{'<NULL>':0} for s in range(self.K)]
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
    
    property K:
        def __get__(self): return self.thisptr.K
    property states:
        def __get__(self): return self.thisptr.states
