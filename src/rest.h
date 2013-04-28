// uncomment to enable memoisation
//#define MEMOISE

struct rest {
    // tables with same value grouped into rooms
    struct room {
        int ntab, ncust; // t_uw, c_uw.: number of tables and customers
        vector<int> tab_ncust; // c_uwk: table sizes

        room() : ntab(0), ncust(0), tab_ncust() {};
    };

#define ALPHA 0 /* concentration param */
    param_t *param;
    rest *parent; // pi(u)
    int ctxt_len; // m = |u|
    int ntab, ncust; // t_u., c_u..: totals
    hash_map<word_t,room> rooms; // map w to room of tables
#ifdef MEMOISE
    mutable hash_map<word_t,double> p_word_mem;
#endif

    rest(param_t *p, rest *par, int l)
            : param(p), parent(par), ctxt_len(l), ntab(0), ncust(0), rooms() {
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

    double p_word_raw(word_t w) const {
        double d = param->discount[ctxt_len];
        double p = 0;
        if(rooms.count(w)) { // word observed previously
            const room &room_w = rooms.find(w)->second;
            p += room_w.ncust - d * room_w.ntab;
        }
        p += (ALPHA + d * ntab) * parent->p_word(w);
        return p;
    }

    double p_word(word_t w) const {
        if(parent == NULL) return uniform_dist(param->vocab_size);
        if(ncust == 0) return parent->p_word(w);

#ifdef MEMOISE
        double p;
        if(p_word_mem.count(w)) // memoise
            p = p_word_mem[w];
        else
            p = p_word_mem[w] = p_word_raw(w);
#else
        double p = p_word_raw(w);
#endif
        return p / (ALPHA + ncust);
    }

    double cdf(word_t w) const {
        if(w >= param->vocab_size)
            w = param->vocab_size - 1;
        double p = 0;
        for(word_t i = 0; i <= w; i++)
            p += p_word(i);
        return p;
    }

    word_t sample_word(void) const {
        if(parent == NULL) return uniform_sample(param->vocab_size);

        double d = param->discount[ctxt_len];
        double U = randu(0, ALPHA + ncust);
        for(hash_map<word_t,room>::const_iterator i = rooms.begin();
                i != rooms.end(); i++) {
            word_t w = i->first; const room &room_w = i->second;
            if(room_w.ncust == 0) continue;
            U -= room_w.ncust - d * room_w.ntab;
            if(U <= EPS) return w;
        }

        // sample from parent with probability propto ALPHA + d * ntab
        return parent->sample_word();
    }

    int sample_tab(word_t w, room &room_w, bool discnt) {
        int k_new = room_w.tab_ncust.size();
        if(k_new == 0) { // add first table
            room_w.tab_ncust.push_back(0);
            return 0;
        }

        // determine probability normalising const
        double d = param->discount[ctxt_len];
        double norm = room_w.ncust;
        if(discnt) {
            norm -= d * room_w.ntab;
            norm += (ALPHA + d * ntab) * parent->p_word(w);
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
            if(k_new >= room_w.tab_ncust.size())
                room_w.tab_ncust.push_back(0);
            return k_new;
        }

        assert(0); // shouldn't reach here
    }

    void add_cust(word_t w, room &room_w) {
        room_w.ncust++; ncust++;
        if(parent == NULL) // base dist
            return;
#ifdef MEMOISE
        p_word_mem.erase(w);
#endif

        int k = sample_tab(w, room_w, true);
        if(room_w.tab_ncust[k] == 0) { // new table
            parent->add_cust(w);
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

    void rm_cust(word_t w, room &room_w) {
        assert(room_w.ncust > 0);
        room_w.ncust--; ncust--;
        if(parent == NULL) // base dist
            return;
#ifdef MEMOISE
        p_word_mem.erase(w);
#endif

        int k = sample_tab(w, room_w, false);
        room_w.tab_ncust[k]--;
        if(room_w.tab_ncust[k] == 0) { // remove empty table
            parent->rm_cust(w);
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
        }
#ifdef MEMOISE
        p_word_mem.clear();
#endif
    }
};
