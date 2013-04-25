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

struct param_t {
    int N; // N-gram assumption
    double discount[10]; // d_m
    double beta1[10], beta2[10]; // d_m ~ Beta(beta1[m], beta2[m])
    int vocab_size;
    int rest_count;

    param_t(int ngram_size, int nwords)
            : N(ngram_size), vocab_size(nwords), rest_count(0) {
        for(int i = 0; i < N; i++)
            discount[i] = .5;
    }
};

double randu(double a, double b) { // U(a,b)
    return a + (b-a) * (double) rand() / RAND_MAX;
}

double randbeta(double a, double b) { // Beta(a,b)
    beta_distribution<> dist(a,b);
    return quantile(dist, randu(0,1)); // inverse transform
}

double uniform_dist(int size) {
    return 1.0 / size;
}

int uniform_sample(int size) {
    return rand() % size;
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

#define ALPHA 0 /* concentration param */
    param_t *param;
    rest *parent; // pi(u)
    int ctxt_len; // m = |u|
    int ntab, ncust; // t_u., c_u..: totals
    hash_map<word_t,room> m; // map w to room of tables

    rest(param_t *p, rest *par, int l)
            : param(p), parent(par), ctxt_len(l), ntab(0), ncust(0), m() {
#if !SPARSE_HASH
        m.set_empty_key(-1);
#endif
        assert(ctxt_len < param->N);
    }

    double p_word(word_t w) const {
        if(parent == NULL) return uniform_dist(param->vocab_size);
        if(ncust == 0) return parent->p_word(w);

        double d = param->discount[ctxt_len];
        double p = 0;

        if(m.count(w)) { // word observed previously
            const room &m_w = m.find(w)->second;
            p += m_w.ncust - d * m_w.ntab;
        }
        p += (ALPHA + d * ntab) * parent->p_word(w);
        p /= ALPHA + ncust;
        return p;
    }

    double cdf(word_t w) const {
        double p = 0;
        for(word_t i = 0; i <= w; i++)
            p += p_word(i);
        return p;
    }

    word_t sample_word(void) const {
        if(parent == NULL) return uniform_sample(param->vocab_size);

        double d = param->discount[ctxt_len];
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
        double d = param->discount[ctxt_len];
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

struct corpus {
    hash_map<string,word_t> dict;
    vector<string> vocab;
    vector<word_t> text;
    vector<int> word_len;

    corpus() {
#if !SPARSE_HASH
        dict.set_empty_key("");
#endif
        vocab.push_back("NULL");
    }

    void push_back(word_t w) {
        text.push_back(w);
    }

    void push_back(string s) {
        if(!dict.count(s)) { // new word
            word_t w = dict.size() + 1;
            dict[s] = w;
            vocab.push_back(s);
            assert(vocab[w] == s);
        }
        word_len.push_back(s.length());
        push_back(dict[s]);
    }

    void push_back(char *buf) {
        push_back(string(buf));
    }

    int size(void) {
        return text.size();
    }

    word_t operator[](int i) {
        return text[i];
    }
};

struct ctxt_tree {
    param_t *param;
    rest *r;
    hash_map<word_t,ctxt_tree*> children;

    ctxt_tree(param_t *p, rest *restaurant)
            : param(p), r(restaurant), children() {
#if !SPARSE_HASH
        children.set_empty_key(-1);
#endif
    }

    ctxt_tree *insert(word_t w) { // extend context to left by w
        if(!children.count(w)) {
            rest *r_new = new rest(param, r, r->ctxt_len + 1);
            children[w] = new ctxt_tree(param, r_new);
            param->rest_count++;
        }
        return children[w];
    }

    rest *insert_context(corpus &text, int j) {
        ctxt_tree *ct = this;
        for(int i = 1; i < param->N; i++) {
            if(j-i < 0) break;
            ct = ct->insert(text[j-i]);
        }
        return ct->r;
    }

    const ctxt_tree *get(word_t w) const {
        if(!children.count(w))
            return this;
        return children.find(w)->second;
    }

    const rest *get_context(corpus &text, int j) const {
        const ctxt_tree *ct = this;
        for(int i = 1; i < param->N; i++) {
            if(j-i < 0) break;
            word_t w = text[j-i];
            if(!ct->children.count(w)) break; // can't extend context further
            ct = ct->get(w);
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
        double d = param->discount[m];
        if(r->ntab > 0) param->beta1[m] += r->ntab - 1;

        // update beta2
        for(hash_map<word_t,rest::room>::iterator i = r->m.begin();
                i != r->m.end(); i++) {
            vector<int> &v = i->second.v;
            for(int k = 0; k < v.size(); k++) {
                int ncust = v[k];
                for(int j = 1; j < ncust; j++)
                    if(randu(0,1) < (1-d)/(j-d)) param->beta2[m]++;
            }
        }

        // recur
        for(hash_map<word_t,ctxt_tree*>::iterator i = children.begin();
                i != children.end(); i++)
            i->second->update_beta();
    }

    void resample_param(void) {
        for(int i = 0; i < param->N; i++)
            param->beta1[i] = param->beta2[i] = 1;
        update_beta();
        for(int i = 0; i < param->N; i++)
            param->discount[i] = randbeta(param->beta1[i], param->beta2[i]);
    }

    void resample(void) {
        resample_seat();
        resample_param();
    }
};

void hpylm(int ngram_size, int test_size) {
    // read data
    corpus text;
    char buf[100];
    while(scanf("%s", buf) != EOF)
        text.push_back(buf);
    int train_size = text.size() - test_size;

    printf("%d (out of %d) words in dict\n",
        (int) text.dict.size(), (int) text.size());
    printf("%d training, %d testing\n", train_size, test_size);
    fflush(stdout);

    // init params
    param_t *param = new param_t(ngram_size, text.dict.size());

    // init restaurant context tree
    rest *G_0 = new rest(param, NULL, -1); // base distribution
    rest *G_eps = new rest(param, G_0, 0); // context-free dist
    ctxt_tree root(param, G_eps);

    // add training data
    for(int j = 0; j < train_size; j++) {
        rest *r = root.insert_context(text, j);
        r->add_cust(text[j]);
    }

    printf("%d restaurants created\n", param->rest_count);

    // size of test data
    int test_bytes = 0;
    for(int j = train_size; j < train_size + test_size; j++)
        test_bytes += text.word_len[j] + 1;

    printf("%gkB of test data\n", test_bytes/1e3);
    fflush(stdout);

    // Gibbs sampling
    int iters = 300, burnin = 125;
    double log_sum_prob = log2(0); // log(sum_H P(w|H))
    for(int i = 0; i < iters; i++) {
        printf("ITER %3d: ", i);
        root.resample();
        for(int j = 0; j < param->N; j++)
            printf("d%d = %.3f; ", j, param->discount[j]);

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
    corpus text_sample;
    int sample_size = 250;
    printf("\nSAMPLE TEXT\n... ");
    for(int j = 0; j < sample_size; j++) {
        const rest *r = root.get_context(text_sample, j);
        word_t w = r->sample_word();
        printf("%s ", text.vocab[w].c_str());
        text_sample.push_back(w);
        fflush(stdout);
    }
    printf("...\n");

    // range coding dry run
    double low = 0, range = 1, log_range = 0;
    for(int j = 0; j < sample_size; j++) {
        const rest *r = root.get_context(text_sample, j);
        word_t w = text_sample[j];
        low += range * r->cdf(w-1);
        double p = r->p_word(w); range *= p; log_range += log2(p);
        assert(abs(r->cdf(param->vocab_size-1) - 1) < EPS);
    }
    printf("RANGE approx %.16f + 2^%f\n", low, log_range);
}

void pyphmm(void) {
    corpus words, tags;
    char buf1[100], buf2[100];
    while(scanf("%s %s", buf1, buf2) != EOF) {
        words.push_back(buf1);
        tags.push_back(buf2);
    }

    int test_size = words.size() / 5;
    int train_size = words.size() - test_size;

    param_t *t_param = new param_t(3, tags.dict.size());
    rest *t_G_uniform = new rest(t_param, NULL, -1);
    rest *t_G_unigram = new rest(t_param, t_G_uniform, 0);
    ctxt_tree t_root(t_param, t_G_unigram);

    param_t *w_param = new param_t(2, words.dict.size());
    rest *w_G_uniform = new rest(w_param, NULL, 0);
    ctxt_tree w_root(w_param, w_G_uniform);

    for(int j = 0; j < train_size; j++) {
        rest *t_r = t_root.insert_context(tags, j); t_r->add_cust(tags[j]);
        rest *w_r = w_root.insert(tags[j])->r;      w_r->add_cust(words[j]);
    }

    printf("%d tag, %d word restaurants\n",
            t_param->rest_count, w_param->rest_count);

    int errors = 0;
    corpus best_tags;
    for(int j = train_size; j < train_size + test_size; j++) {
        word_t w = words[j];
        //const rest *t_r = t_root.get_context(tags, j);
        const rest *t_r = t_root.get_context(best_tags, j-train_size);

        // approx posterior over next tag
        vector<double> tag_probs;
        double p_w = 0;
        word_t t_best; double p_best = 0;
        for(word_t t = 0; t < t_param->vocab_size; t++) {
            const rest *w_r = w_root.get(t)->r;
            double p_t   = t_r->p_word(t);
            double p_w_t = w_r->p_word(w);
            double p_joint = p_w_t * p_t;

            tag_probs.push_back(p_joint);
            p_w += p_joint;
            if(p_joint > p_best) {
                t_best = t;
                p_best = p_joint;
            }
        }

        word_t t = tags[j];
        double p_t_w = tag_probs[t] / p_w;
        if(t_best != t) errors++;
        best_tags.push_back(t_best);

        if(j < train_size + 250) {
            printf("%s/%s", words.vocab[w].c_str(), tags.vocab[t].c_str());
            if(t_best != t) printf("(%s)", tags.vocab[t_best].c_str());
            printf(" ");
            fflush(stdout);
        }
    }

    printf("\n\n%f%% error rate\n", 100.0 * errors / test_size);
}

int main(int argc, char **argv) {
#ifdef HPYLM
    assert(argc == 3);
    hpylm(atoi(argv[1]), atoi(argv[2]));
#endif
#ifdef PYPHMM
    pyphmm();
#endif
    return 0;
}
