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
            if(discnt) U += d;
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
