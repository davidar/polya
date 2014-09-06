#pragma once

#include <vector>
#include <string>

#include "util.h"
#include "rand.h"
#include "Exchangeable.h"
#include "LogR.h"
#include "DynDict.h"
#include "Trie.h"

class HMM {
    typedef N S; // state space
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
    std::vector<X> text;
    
    HMM(N context_len, N num_states, std::vector<X> text,
        std::vector<Exchangeable*> emit, T::F f, void *user_data)
        : M(context_len), K(num_states),
          text(text), emit(emit), transition(f,user_data) {}
    
    void train(std::vector<S> ss) {
        states = ss;
        FOR(t, text.size()) {
            t_observe(t,states[t]);
            e_observe(t,states[t]);
        }
    }
    
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
    
    R resample(std::vector<N> ts) {
#define FOR_TGRAMS \
            FOR(i,ts.size()) \
                LET(N start = (i == 0 || horz(ts[i-1]) <= ts[i]) \
                    ? ts[i] : horz(ts[i-1])) \
                FOR(t, start,horz(ts[i]))
        LogR joint(1);
        
        // forget old states
        for(N t : ts) e_forget(t, states[t]);
        FOR_TGRAMS t_forget(t, states[t]);
        
        // posterior
        LogR lpost[K]; LogR norm = 0;
        FOR(s, K) {
            LogR p = 1;
            for(N t : ts) {
                states[t] = s;
                p *= e_predict(t,s);
                e_observe(t,s);
            }
            FOR_TGRAMS {
                p *= t_predict(t, states[t]);
                t_observe(t, states[t]);
            }
            norm += lpost[s] = p;
            
            // rollback
            for(N t : ts) e_forget(t, states[t]);
            FOR_TGRAMS t_forget(t, states[t]);
        }
        
        // sample new state
        R post[K]; FOR(s, K) post[s] = (R)(lpost[s] / norm);
        S s = sample(post, K);
        for(N t : ts) {
            states[t] = s;
            joint *= post[s];
        }
        
        // update model
        for(N t : ts) e_observe(t, states[t]);
        FOR_TGRAMS t_observe(t, states[t]);
        return log(joint);
    }
    
    R predict(std::vector<X> xs) {
        assert(M == 2); // TODO: generalise
        // alpha[i][j] = p(s_n = j, s_n-1 = i | x_1..n)
        R alpha[K][K]; memset(alpha, 0, sizeof alpha);
        alpha[0][0] = 1;
        LogR prob(1);
        for(X x : xs) {
            R alpha_new[K][K]; memset(alpha_new, 0, sizeof alpha_new);
            R px = 0;
            FOR(k,K) {
                R p_emit = emit[k]->predict(x);
                FOR(j,K) {
                    R p = 0;
                    FOR(i,K) p += transition[{i,j}]->predict(k) * alpha[i][j];
                    p *= p_emit;
                    px += alpha_new[j][k] = p;
                }
            }
            prob *= px;
            FOR(k,K) FOR(j,K) alpha[j][k] = alpha_new[j][k] / px;
        }
        return log(prob);
    }
    
    R predict(std::string text, std::vector<Trie*> tries, const N maxr) {
        assert(M == 2); // TODO: generalise
        LogR prob(1);
        // alpha[r][i][j] = p(word started at t-r, s_t = j, s_t-1 = i | x_1..t)
        R alpha[maxr][K][K]; memset(alpha, 0, sizeof alpha);
        
        char c = text[0];
        R pc = 0;
        FOR(k,K) pc += alpha[0][0][k] =
            tries[k]->predict(c) * transition[{}]->predict(k);
        FOR(k,K) alpha[0][0][k] /= pc;
        prob *= pc;
        
        FOR(t, 1,text.size()) {
            c = text[t];
            R alpha_new[maxr][K][K]; memset(alpha_new, 0, sizeof alpha_new);
            pc = 0;
            FOR(k,K) FOR(j,K) {
                FOR(r, 1,maxr) {
                    auto start = text.begin() + t - r;
                    if(alpha[r-1][j][k] > 0) {
                        const Trie *triek = tries[k]->lookup(start, r);
                        if(triek == NULL) alpha_new[r][j][k] = 0;
                        else alpha_new[r][j][k] =
                                alpha[r-1][j][k] * triek->predict(c);
                        pc += alpha_new[r][j][k];
                    }
                    
                    const Trie *triej = tries[j]->lookup(start, r);
                    FOR(i,K) if(alpha[r-1][i][j] > 0 && triej != NULL) {
                        R a = alpha[r-1][i][j]
                            * triej->predict()
                            * transition[{i,j}]->predict(k)
                            * tries[k]->predict(c);
                        alpha_new[0][j][k] += a;
                    }
                }
                pc += alpha_new[0][j][k];
            }
            
            FOR(k,K) FOR(j,K) FOR(r,maxr)
                alpha[r][j][k] = alpha_new[r][j][k] / pc;
            prob *= pc;
        }
        return log(prob);
    }
};
