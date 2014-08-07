from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.vector cimport vector
from cpython.exc cimport PyErr_CheckSignals

cimport _polya
from _polya cimport R, N, X

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
    cdef _polya.Exchangeable *thisptr
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
