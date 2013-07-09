// uncomment to enable memoisation
//#define MEMOISE

struct rest {
    // tables with same value grouped into rooms
    struct room {
        int ntab, ncust; // t_uw, c_uw.: number of tables and customers
        vector<int> tab_ncust; // c_uwk: table sizes
        vector<int> tab_floor; // which parent/floor originated this table

        room() : ntab(0), ncust(0) {};
    };

    param_t *param;
    const int ctxt_len; // m = |u|

    rest **parents; // pi(u)
    int nparents; // |pi(u)|
    rest *lambda; // lambda_w->v dist

    int ntab, ncust; // t_u., c_u..: totals
    hash_map<word_t,room> rooms; // map w to room of tables

#ifdef MEMOISE
    mutable hash_map<word_t,double> p_word_mem;
#endif

    void init(void) {
        ntab = ncust = 0;
#if !SPARSE_HASH
        rooms.set_empty_key(-1);
# ifdef MEMOISE
        p_word_mem.set_empty_key(-1);
# endif
#endif
#ifdef MEMOISE
        p_word_mem.set_deleted_key(-2);
#endif
        assert(ctxt_len < param->N);
    }

    rest(param_t *p, rest *par, int l)
            : param(p), ctxt_len(l) {
        if(par == NULL) {
            parents = NULL;
            nparents = 0;
        } else {
            parents = new rest *[1];
            parents[0] = par;
            nparents = 1;
        }
        lambda = NULL;
        init();
    }

    rest(param_t *p, int l, rest *pars[], int npars, rest *lam)
            : param(p), ctxt_len(l),
              parents(pars), nparents(npars), lambda(lam) {
        init();
    }

    double parent_p_word(word_t w) const {
        if(nparents == 1)
            return parents[0]->p_word(w);

        // mixture
        double p = 0;
        for(int i = 0; i < nparents; i++)
            p += lambda->p_word(i) * parents[i]->p_word(w);
        return p;
    }

    double p_word_raw(word_t w) const {
        const double d = param->disc[ctxt_len], a = param->conc[ctxt_len];
        double p = 0;
        if(rooms.count(w)) { // word observed previously
            const room &room_w = rooms.find(w)->second;
            p += room_w.ncust - d * room_w.ntab;
        }
        p += (a + d * ntab) * parent_p_word(w);
        return p;
    }

    double p_word(word_t w) const {
        if(nparents == 0) return uniform_dist(param->vocab_size);
        if(ncust == 0) return parent_p_word(w);

        const double a = param->conc[ctxt_len];
#ifdef MEMOISE
        double p;
        if(p_word_mem.count(w)) // memoise
            p = p_word_mem[w];
        else
            p = p_word_mem[w] = p_word_raw(w);
#else
        double p = p_word_raw(w);
#endif
        return p / (a + ncust);
    }

    double cdf(word_t w) const {
        if(w >= param->vocab_size)
            w = param->vocab_size - 1;
        double p = 0;
        for(word_t i = 0; i <= w; i++)
            p += p_word(i);
        return p;
    }

    word_t parent_sample_word(void) const {
        assert(parents != NULL);
        if(nparents == 1)
            return parents[0]->sample_word();

        // mixture
        int i = lambda->sample_word();
        return parents[i]->sample_word();
    }

    word_t sample_word(void) const {
        if(nparents == 0) return uniform_sample(param->vocab_size);

        const double d = param->disc[ctxt_len], a = param->conc[ctxt_len];
        double U = randu(0, a + ncust);
        for(hash_map<word_t,room>::const_iterator i = rooms.begin();
                i != rooms.end(); i++) {
            word_t w = i->first; const room &room_w = i->second;
            if(room_w.ncust == 0) continue;
            U -= room_w.ncust - d * room_w.ntab;
            if(U <= EPS) return w;
        }

        // sample from parent with probability propto a + d * ntab
        return parent_sample_word();
    }

    int sample_tab(word_t w, room &room_w, bool discnt) {
        int k_new = room_w.tab_ncust.size();
        if(k_new == 0) { // add first table
            room_w.tab_ncust.push_back(0);
            room_w.tab_floor.push_back(0);
            return 0;
        }

        // determine probability normalising const
        const double d = param->disc[ctxt_len], a = param->conc[ctxt_len];
        double norm = room_w.ncust;
        if(discnt) {
            norm -= d * room_w.ntab;
            norm += (a + d * ntab) * parent_p_word(w);
        }

        // sample
        double U = randu(0, norm);
        for(int k = 0; k < room_w.tab_ncust.size(); k++) {
            int tabsize = room_w.tab_ncust[k];
            if(tabsize == 0) { // unoccupied table
                k_new = k;
                continue;
            }
            U -= tabsize;
            if(discnt) U += d;
            if(U <= EPS) return k;
        }

        if(discnt) { // new table
            if(k_new >= room_w.tab_ncust.size()) {
                room_w.tab_ncust.push_back(0);
                room_w.tab_floor.push_back(0);
            }
            return k_new;
        }

        assert(0); // shouldn't reach here
    }

    int parent_add_cust(word_t w) {
        assert(parents != NULL);
        if(nparents == 1) {
            parents[0]->add_cust(w);
            return 0;
        }

        // sample parent/floor from posterior given word
        double probs[nparents], norm = 0;
        for(int i = 0; i < nparents; i++) {
            double p = lambda->p_word(i) * parents[i]->p_word(w);
            probs[i] = p; norm += p;
        }

        int floor = sample_simple(nparents, probs, norm);
        lambda->add_cust(floor);
        parents[floor]->add_cust(w);
        return floor;
    }

    void add_cust(word_t w, room &room_w) {
        room_w.ncust++; ncust++;
        if(nparents == 0) // base dist
            return;
#ifdef MEMOISE
        p_word_mem.erase(w);
#endif

        int k = sample_tab(w, room_w, true);
        if(room_w.tab_ncust[k] == 0) { // new table
            int floor = parent_add_cust(w);
            room_w.tab_floor[k] = floor;
            room_w.ntab++; ntab++;
#ifdef MEMOISE
            p_word_mem.clear();
#endif
        }
        room_w.tab_ncust[k]++;
    }

    void add_cust(word_t w) {
        add_cust(w, rooms[w]);
    }

    void parent_rm_cust(word_t w, int floor) {
        assert(parents != NULL);
        if(nparents == 1) {
            parents[0]->rm_cust(w);
            return;
        }

        // remove table from floor
        lambda->rm_cust(floor);
        parents[floor]->rm_cust(w);
    }

    void rm_cust(word_t w, room &room_w) {
        assert(room_w.ncust > 0);
        room_w.ncust--; ncust--;
        if(nparents == 0) // base dist
            return;
#ifdef MEMOISE
        p_word_mem.erase(w);
#endif

        int k = sample_tab(w, room_w, false);
        room_w.tab_ncust[k]--;
        if(room_w.tab_ncust[k] == 0) { // remove empty table
            parent_rm_cust(w, room_w.tab_floor[k]);
            room_w.ntab--; ntab--;
#ifdef MEMOISE
            p_word_mem.clear();
#endif
        }
    }

    void rm_cust(word_t w) {
        rm_cust(w, rooms[w]);
    }

    void resample(void) { // remove and re-add all customers
        for(hash_map<word_t,room>::iterator i = rooms.begin();
                i != rooms.end(); i++) {
            word_t w = i->first; room &room_w = i->second;
            for(int j = 0; j < room_w.ncust; j++) {
                rm_cust(w, room_w);
                add_cust(w, room_w);
            }
            for(int k = 0; k < room_w.ntab; k++) {
                if(room_w.tab_ncust[k] == 0) continue;
                parent_rm_cust(w, room_w.tab_floor[k]);
                room_w.tab_floor[k] = parent_add_cust(w);
            }
        }
#ifdef MEMOISE
        p_word_mem.clear();
#endif
    }

    void update_hyperparam(void) {
        int m = ctxt_len;
        const double d = param->disc[ctxt_len], a = param->conc[ctxt_len];

        for(int i = 1; i < ntab; i++) {
            if(randu(0,1) < a/(a + d*i))
                param->gamma1[m]++;
            else
                param->beta1[m]++;
        }

        // iter over indiv. tables
        for(hash_map<word_t,room>::iterator i = rooms.begin();
                i != rooms.end(); i++) {
            vector<int> &tab_ncust = i->second.tab_ncust;
            for(int k = 0; k < tab_ncust.size(); k++) {
                int ncust = tab_ncust[k];

                for(int j = 1; j < ncust; j++)
                    if(randu(0,1) < (1-d)/(j-d))
                        param->beta2[m]++;
            }
        }

        if(ncust > 1)
            param->gamma2[m] -= log(randbeta(a+1, ncust-1)); 
    }
};
