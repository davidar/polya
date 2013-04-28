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
        for(hash_map<word_t,rest::room>::iterator i = r->rooms.begin();
                i != r->rooms.end(); i++) {
            vector<int> &tab_ncust = i->second.tab_ncust;
            for(int k = 0; k < tab_ncust.size(); k++) {
                int ncust = tab_ncust[k];
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
