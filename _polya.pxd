from libcpp.string cimport string
from libcpp.vector cimport vector

ctypedef double R
ctypedef unsigned int N
ctypedef unsigned int X

cdef extern from "util.h" nogil:
    R cputime()

cdef extern from "Exchangeable.h" nogil:
    cdef cppclass Exchangeable:
        R predict(X)
        X sample()
        Exchangeable& observe(X)
        Exchangeable& forget(X)
        void resample()
        string toString()

cdef extern from "Uniform.h" nogil:
    cdef cppclass Uniform(Exchangeable):
        N n
        Uniform(N)
        
cdef extern from "Polya.h" nogil:
    cdef cppclass Polya(Exchangeable):
        cppclass Hyper:
            R a, d
            Hyper()
            Hyper(string)
            void resample()
        Polya(Hyper&, Exchangeable&)

ctypedef Polya.Hyper PolyaHyper

cdef extern from "Mixture.h" nogil:
    cdef cppclass Mixture(Exchangeable):
        Mixture(Exchangeable&, vector[Exchangeable*])

cdef extern from "Odds.h" nogil:
    cdef cppclass Odds(Exchangeable):
        cppclass Term:
            R t
            Term() except +
            void resample()
        Odds(vector[Term*]) except +

ctypedef Odds.Term OddsTerm
