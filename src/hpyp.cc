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

// undef to fix predictions during testing
//#define TEST_UPDATE

int N = 0; // N-gram assumption
double discount[10]; // d_m
double beta1[10], beta2[10]; // d_m ~ Beta(beta1[m], beta2[m])
const double ALPHA = 0; // concentration param

// uniform base distribution
int vocab_size = 0;
#define BASE_DIST(w) (1.0 / vocab_size)
#define BASE_SAMPLE() (rand() % vocab_size)

int rest_count = 0;

double randu(double a, double b) { // U(a,b)
    return a + (b-a) * (double) rand() / RAND_MAX;
}

double randbeta(double a, double b) { // Beta(a,b)
    beta_distribution<> dist(a,b);
    return quantile(dist, randu(0,1)); // inverse transform
}

double log2_add(double log_a, double log_b) { // log(a),log(b) -> log(a+b)
    if(log_a < log_b) return log2_add(log_b, log_a); // ensure log_a >= log_b
    return log_a + log2(1 + exp2(log_b - log_a)); // log(a(1+b/a))
}

double log2_sub(double log_a, double log_b) { // log(a-b)
    return log_a + log2(1 - exp2(log_b - log_a)); // log(a(1-b/a))
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
        if(parent == NULL) return BASE_DIST(w);
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

    word_t sample_word(void) const {
        if(parent == NULL) return BASE_SAMPLE();

        double d = discount[ctxt_len];
        double U = randu(0, ALPHA + ncust);
        for(hash_map<word_t,room>::const_iterator i = m.begin();
                i != m.end(); i++) {
            word_t w = i->first; const room &m_w = i->second;
            if(m_w.ncust == 0) continue;
            U -= m_w.ncust - d * m_w.ntab;
            if(U <= EPS) return w;
        }

        // sample from parent with probability propto ALPHA + d * ntab
        return parent->sample_word();
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
        for(hash_map<word_t,room>::iterator i = m.begin();
                i != m.end(); i++) {
            word_t w = i->first; room &m_w = i->second;
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

    ctxt_tree(rest *restaurant) : r(restaurant), children() {
#if !SPARSE_HASH
        children.set_empty_key(-1);
#endif
    }

    ctxt_tree *insert(word_t w) { // extend context to left by w
        if(!children.count(w)) {
            rest *r_new = new rest(r, r->ctxt_len + 1);
            children[w] = new ctxt_tree(r_new);
            rest_count++;
        }
        return children[w];
    }

    rest *insert_context(vector<word_t> &text, int j) {
        ctxt_tree *ct = this;
        for(int i = 1; i < N; i++) {
            if(j-i < 0) break;
            ct = ct->insert(text[j-i]);
        }
        return ct->r;
    }

    const rest *get_context(vector<word_t> &text, int j) const {
        const ctxt_tree *ct = this;
        for(int i = 1; i < N; i++) {
            if(j-i < 0) break;
            word_t w = text[j-i];
            if(!ct->children.count(w)) break; // can't extend context further
            ct = ct->children.find(w)->second;
        }
        return ct->r;
    }

    void resample_seat(void) { // reseat all customers
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

    void resample_param(void) {
        for(int i = 0; i < N; i++)
            beta1[i] = beta2[i] = 1;
        update_beta();
        for(int i = 0; i < N; i++)
            discount[i] = randbeta(beta1[i], beta2[i]);
    }

    void resample(void) {
        resample_seat();
        resample_param();
    }
};

int main(int argc, char **argv) {
    assert(argc == 3);

    // read data
    hash_map<string,word_t> dict;
#if !SPARSE_HASH
    dict.set_empty_key("");
#endif
    vector<string> vocab; vocab.push_back("NULL");
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
            vocab.push_back(s);
            assert(vocab[w] == s);
        }
        text.push_back(dict[s]);
        word_len.push_back(strlen(buf));
    }
    printf("%d (out of %d) words in dict\n", (int) dict.size(), (int) text.size());
    int test_size = atoi(argv[2]);
    int train_size = text.size() - test_size;
    printf("%d training, %d testing\n", train_size, test_size);
    fflush(stdout);

    // init params
    N = atoi(argv[1]);
    for(int i = 0; i < N; i++)
        discount[i] = .5;
    vocab_size = dict.size();

    // init restaurant context tree
    rest *G_0 = new rest(NULL, -1); // base distribution
    rest *G_eps = new rest(G_0, 0); // context-free dist
    ctxt_tree root(G_eps);

    // add training data
    for(int j = N; j < train_size; j++) {
        rest *r = root.insert_context(text, j);
        r->add_cust(text[j]);
    }
    printf("%d restaurants created\n", rest_count);

    // size of test data
    int test_bytes = 0;
    for(int j = train_size; j < train_size + test_size; j++)
        test_bytes += word_len[j] + 1;
    printf("%gkB of test data\n", test_bytes/1e3);
    fflush(stdout);

    // Gibbs sampling
    int iters = 300, burnin = 125;
    double log_sum_prob = log2(0); // log(sum_H P(w|H))
    for(int i = 0; i < iters; i++) {
        printf("ITER %3d: ", i);
        root.resample();
        for(int j = 0; j < N; j++) printf("d%d = %.3f; ", j, discount[j]);

        // calculate likelihood of test data
        double bits = 0;
        for(int j = train_size; j < train_size + test_size; j++) {
#ifdef TEST_UPDATE
            rest *r = root.insert_context(text, j);
#else
            const rest *r = root.get_context(text, j);
#endif
            double p = r->p_word(text[j]);
            bits += -log2(p);
#ifdef TEST_UPDATE
            r->add_cust(text[j]);
#endif
        }
        printf("PPLX 2^%.2f = %.1f\n", bits/test_size, exp2(bits/test_size));
        fflush(stdout);

#ifdef TEST_UPDATE
        // remove test data for next iteration
        for(int j = train_size; j < train_size + test_size; j++) {
            rest *r = root.insert_context(text, j);
            r->rm_cust(text[j]);
        }
#endif

        if(i >= burnin) // average predictions
            log_sum_prob = log2_add(log_sum_prob, -bits);
    }

    printf("\nSUMMARY\n");
    double log_mean = log_sum_prob - log2(iters - burnin); // log(sum P/N)
    printf("Compression ratio: %f bits/char (%f%%)\n", -log_mean / test_bytes,
            100.0 * -log_mean / (8*test_bytes));
    printf("Perplexity: 2^%f = %f\n",
            -log_mean / test_size, exp2(-log_mean / test_size));
    fflush(stdout);

    // sample text from model
    vector<word_t> text_sample;
    int sample_size = 250;
    printf("\nSAMPLE TEXT\n... ");
    for(int j = 0; j < sample_size; j++) {
        const rest *r = root.get_context(text_sample, j);
        word_t w = r->sample_word();
        printf("%s ", vocab[w].c_str());
        text_sample.push_back(w);
        fflush(stdout);
    }
    printf("...\n");

    return 0;
}
