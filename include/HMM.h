#pragma once

#include <vector>

#include "util.h"
#include "rand.h"
#include "Exchangeable.h"
#include "LogR.h"
#include "DynDict.h"

typedef N S; // state space

class AbstractHMM {
    protected:
    virtual void e_observe(N t, S s) = 0; // observe emission
    virtual void e_forget(N t, S s) = 0; // forget emission
    virtual R e_predict(N t, S s) = 0;  // predict emission
    
    virtual void t_observe(N t, S s) = 0; // observe transition
    virtual void t_forget(N t, S s) = 0; // forget transition
    virtual R t_predict(N t, S s) = 0;  // predict transition
    
    inline N horz(N t) {
        // index of next state independent of the current position t
        N z = t + M + 1;
        if(z > states.size()) return states.size();
        return z;
    }
    
    R resample_state(N t) {
        // posterior distribution over states
        R post[K]; R norm = 0;
        FOR(s, K) {
            states[t] = s;
            R p = e_predict(t,s);
            FOR(i, t,horz(t)) p *= t_predict(i, states[i]);
            norm += post[s] = p;
        }
        FOR(s, K) post[s] /= norm;
        
        // sample new state from posterior
        states[t] = sample(post, K);
        return post[states[t]];
    }
    
    public:
    const N M, K; // context length, maximum number of states
    std::vector<S> states;
    
    AbstractHMM(N context_len, N num_states) : M(context_len), K(num_states) {}
    
    R resample() {
        LogR post(1);
        FOR(t, states.size()) {
            // forget old state, and states depending on it
            e_forget(t, states[t]);
            FOR(i, t,horz(t)) t_forget(i, states[i]);
            
            post *= resample_state(t);
            
            // update model
            e_observe(t, states[t]);
            FOR(i, t,horz(t)) t_observe(i, states[i]);
        }
        return log(post);
    }
};

class HMM : public AbstractHMM {
    std::vector<X> text;
    std::vector<Exchangeable*> emit;
    typedef DynDict<std::vector<X>,Exchangeable*> T;
    T transition;
    
    void e_observe(N t, S s)  { emit[s]->observe(text[t]); }
    void e_forget (N t, S s)  { emit[s]->forget (text[t]); }
    R e_predict   (N t, S s) DO(emit[s]->predict(text[t]))
    
    Exchangeable *trans(N t) {
        N start = (t<M) ? 0 : t-M;
        std::vector<S> u(states.begin() + start, states.begin() + t);
        return transition[u];
    }
    void t_observe(N t, S s)  { trans(t)->observe(s); }
    void t_forget (N t, S s)  { trans(t)->forget (s); }
    R t_predict   (N t, S s) DO(trans(t)->predict(s))
    
    public:
    HMM(N context_len, N num_states, std::vector<X> text,
        std::vector<Exchangeable*> emit, T::F f, void *user_data)
        : AbstractHMM(context_len, num_states),
          text(text), emit(emit), transition(f,user_data) {}
    
    void train() {
        FOR(t, text.size()) {
            S s = rand() % K;
            states.push_back(s);
            t_observe(t,s);
            e_observe(t,s);
        }
    }
};
