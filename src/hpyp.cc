// 2013 David A Roberts

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <vector>
#include <string>
using namespace std;

#define SPARSE_HASH 1
#if SPARSE_HASH
#include <sparsehash/sparse_hash_map>
using google::sparse_hash_map;
#define hash_map sparse_hash_map
#else
#include <sparsehash/dense_hash_map>
using google::dense_hash_map;
#define hash_map dense_hash_map
#endif

#include <boost/math/distributions.hpp>
using namespace boost::math;

#define EPS 1e-6

typedef int word_t;

const int N = 5; // N-gram assumption
double discount[N]; // d_m
double beta1[N], beta2[N]; // d_m ~ Beta(beta1[m], beta2[m])
const double ALPHA = 0; // concentration param
double base_dist; // uniform G_0

double randu(double a, double b) { // U(a,b)
    return a + (b-a) * (double) rand() / RAND_MAX;
}

struct rest {
    // tables with same value grouped into rooms
    struct room {
        int ntab, ncust; // t_uw, c_uw.: number of tables and customers
        vector<int> v; // c_uwk: table sizes

        room() : ntab(0), ncust(0), v() {};
    };

    struct rest *parent; // pi(u)
    int ctxt_len; // m = |u|
    int ntab, ncust; // t_u., c_u..: totals
    hash_map<word_t,room> m; // map w to room of tables

    rest(rest *p, int l) : parent(p), ctxt_len(l), ntab(0), ncust(0), m() {
#if !SPARSE_HASH
        m.set_empty_key(-1);
#endif
        assert(ctxt_len < N);
    }

    double p_word(word_t w) const {
        if(parent == NULL) return base_dist;
        if(ncust == 0) return parent->p_word(w);

        double d = discount[ctxt_len];
        double p = 0;

        if(m.count(w)) { // word observed previously
            const room &m_w = m.find(w)->second;
            p += m_w.ncust - d * m_w.ntab;
        }
        p += (ALPHA + d * ntab) * parent->p_word(w);
        p /= ALPHA + ncust;
        return p;
    }

    int sample_tab(word_t w, room &m_w, bool discnt) {
        vector<int> &v = m_w.v;
        int k_new = v.size();

        if(k_new == 0) { // add first table
            v.push_back(0);
            return 0;
        }

        // determine probability normalising const
        double d = discount[ctxt_len];
        double norm = m_w.ncust;
        if(discnt) {
            norm -= d * m_w.ntab;
            norm += (ALPHA + d * ntab) * parent->p_word(w);
        }

        // sample
        double U = randu(0, norm);
        for(int k = 0; k < v.size(); k++) {
            int tabsize = v[k];
            if(tabsize == 0) { // unoccupied table
                k_new = k;
                continue;
            }
            U -= tabsize;
            if(discnt) U -= d;
            if(U <= EPS) return k;
        }

        if(discnt) { // new table
            if(k_new >= v.size()) v.push_back(0);
            return k_new;
        }

        assert(0); // shouldn't reach here
    }

    void add_cust(word_t w, room &m_w) {
        m_w.ncust++; ncust++;
        if(parent == NULL) // base dist
            return;

        int k = sample_tab(w, m_w, true);
        if(m_w.v[k] == 0) { // new table
            parent->add_cust(w);
            m_w.ntab++; ntab++;
        }
        m_w.v[k]++;
    }

    void add_cust(word_t w) {
        add_cust(w, m[w]);
    }

    void rm_cust(word_t w, room &m_w) {
        m_w.ncust--; ncust--;
        if(parent == NULL) // base dist
            return;

        int k = sample_tab(w, m_w, false);
        m_w.v[k]--;
        if(m_w.v[k] == 0) { // remove empty table
            parent->rm_cust(w);
            m_w.ntab--; ntab--;
        }
    }

    void rm_cust(word_t w) {
        rm_cust(w, m[w]);
    }

    void resample(void) { // remove and re-add all customers
        for(hash_map<word_t,room>::iterator i = m.begin(); i != m.end(); i++) {
            word_t w = i->first;
            room &m_w = i->second;
            for(int j = 0; j < m_w.ncust; j++) {
                rm_cust(w, m_w);
                add_cust(w, m_w);
            }
        }
    }
};

struct ctxt_tree {
    rest *r;
    hash_map<word_t,ctxt_tree*> children;

    ctxt_tree() : children() {
#if !SPARSE_HASH
        children.set_empty_key(-1);
#endif
    }

    ctxt_tree *get(word_t w) { // extend context to left by w
        if(!children.count(w)) {
            children[w] = new ctxt_tree;
            children[w]->r = new rest(r, r->ctxt_len + 1);
        }
        return children[w];
    }

    void resample(void) { // reseat all customers
        r->resample();
        for(hash_map<word_t,ctxt_tree*>::iterator i = children.begin();
                i != children.end(); i++)
            i->second->resample();
    }

    void update_beta(void) { // update discount posterior
        int m = r->ctxt_len;
        double d = discount[m];
        if(r->ntab > 0) beta1[m] += r->ntab - 1;

        // update beta2
        for(hash_map<word_t,rest::room>::iterator i = r->m.begin();
                i != r->m.end(); i++) {
            vector<int> &v = i->second.v;
            for(int k = 0; k < v.size(); k++) {
                int ncust = v[k];
                for(int j = 1; j < ncust; j++)
                    if(randu(0,1) < (1-d)/(j-d)) beta2[m]++;
            }
        }

        // recur
        for(hash_map<word_t,ctxt_tree*>::iterator i = children.begin();
                i != children.end(); i++)
            i->second->update_beta();
    }
};

void resample_param(ctxt_tree &root) {
    for(int i = 0; i < N; i++) beta1[i] = beta2[i] = 1;
    root.update_beta();
    for(int i = 0; i < N; i++)
        discount[i] = quantile(beta_distribution<>(beta1[i], beta2[i]), randu(0,1));
}

int main(void) {
    ctxt_tree root;
    root.r = new rest(new rest(NULL, -1), 0);

    // read data
    hash_map<string,int> dict;
#if !SPARSE_HASH
    dict.set_empty_key("");
#endif
    vector<word_t> text;
    vector<int> word_len;
    for(int i = 0; i < N; i++) { // nulls at beginning of text
        text.push_back(0);
        word_len.push_back(0);
    }
    char buf[100];
    while(scanf("%s", buf) != EOF) {
        string s(buf);
        if(!dict.count(s)) { // new word
            word_t w = dict.size() + 1;
            dict[s] = w;
        }
        text.push_back(dict[s]);
        word_len.push_back(strlen(buf));
    }
    printf("%d (out of %d) words in dict\n", (int) dict.size(), (int) text.size());
    int train_size = (int)(.85 * text.size());
    int test_size = text.size() - train_size;
    printf("%d training, %d testing\n", train_size, test_size);

    // init params
    for(int i = 0; i < N; i++)
        discount[i] = .5;
    base_dist = 1.0 / dict.size();

    // add training data
    for(int j = N; j < train_size; j++) {
        rest *r = root.get(text[j-1])->get(text[j-2])->r;
        //rest *r = root.get(text[j-1])->get(text[j-2])->get(text[j-3])->r;
        r->add_cust(text[j]);
    }

    // init test contexts
    int test_bytes = 0;
    for(int j = train_size; j < train_size + test_size; j++) {
        root.get(text[j-1])->get(text[j-2]);
        //root.get(text[j-1])->get(text[j-2])->get(text[j-3]);
        test_bytes += word_len[j] + 1;
    }

    int nsamples = 300, burnin = 125;
    for(int i = 0; i < nsamples; i++) {
        root.resample();
        resample_param(root);
        printf("\niter %d:\td0 = %f, d1 = %f, d2 = %f, d3 = %f\n",
                i, discount[0], discount[1], discount[2], discount[3]);
        if(i < burnin) continue;

        double bits = 0;
        for(int j = train_size; j < train_size + test_size; j++) {
            rest *r = root.get(text[j-1])->get(text[j-2])->r;
            //rest *r = root.get(text[j-1])->get(text[j-2])->get(text[j-3])->r;
            double p = r->p_word(text[j]);
            bits += -log2(p);
        }
        printf("%d / %d = %f bits/char\n", (int) bits, test_bytes, bits / test_bytes);
        printf("perplexity = 2^%f = %f\n", bits / test_size, exp2(bits / test_size));
    }

    return 0;
}
