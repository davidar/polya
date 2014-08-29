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

ctypedef vector[unsigned int] nseq
ctypedef Exchangeable* (*xseq_cb)(nseq, void*)

cdef extern from "LM.h":
    cdef cppclass LM(Exchangeable):
        LM(N, xseq_cb, void*)
        X add_to_dict(nseq)
        R predict_log(N)

cdef extern from "HMM.h":
    cdef cppclass HMM:
        N M, K
        nseq states, text
        HMM(N, N, nseq, vector[Exchangeable*], xseq_cb, void*) except +
        void train(nseq) except +
        R resample() except +
        R resample(nseq) except +
        R predict(nseq) except +
