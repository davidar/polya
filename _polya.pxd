from libcpp.string cimport string
from libcpp.vector cimport vector

ctypedef double R
ctypedef unsigned int N
ctypedef unsigned int X

cdef extern from "util.h" nogil:
    void init()
    R cputime()

cdef extern from "rand.h" nogil:
    R randu(R,R)

cdef extern from "Exchangeable.h":
    cdef cppclass Exchangeable:
        R predict(X) except +
        X sample() except +
        void observe(X) except +
        void forget(X) except +
        void resample() except +
        string toString()

cdef extern from "Uniform.h" nogil:
    cdef cppclass Uniform(Exchangeable):
        N n
        Uniform(N)
        
cdef extern from "Polya.h" nogil:
    cdef cppclass Polya(Exchangeable):
        cppclass Hyper:
            R a, d
            Hyper() except +
            Hyper(string) except +
            void resample() except +
        N c, t
        Polya(Hyper&, Exchangeable&) except +

ctypedef Polya.Hyper PolyaHyper

cdef extern from "Mixture.h" nogil:
    cdef cppclass Mixture(Exchangeable):
        Mixture(Exchangeable&, vector[Exchangeable*]) except +

cdef extern from "Odds.h" nogil:
    cdef cppclass Odds(Exchangeable):
        cppclass Term:
            R t
            Term() except +
            void resample() except +
        Odds(vector[Term*]) except +

ctypedef Odds.Term OddsTerm

ctypedef Exchangeable* (*HMMTF)(vector[unsigned int], void*)

cdef extern from "HMM.h":
    cdef cppclass AbstractHMM:
        N M, K
        vector[unsigned int] states
        R resample() except +
    cdef cppclass HMM(AbstractHMM):
        HMM(N, N, vector[unsigned int], vector[Exchangeable*], HMMTF, void*) except +
        void train() except +
