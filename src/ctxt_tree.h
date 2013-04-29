struct ctxt_tree {
    param_t *param;
    rest *r;
    hash_map<word_t,ctxt_tree*> children;

    ctxt_tree *latent;
    rest **lambda;

    void init(void) {
#if !SPARSE_HASH
        children.set_empty_key(-1);
#endif
    }

    ctxt_tree(param_t *p) : param(p), latent(NULL) {
        rest *uniform_dist = new rest(param, NULL, -1);
        r = new rest(param, uniform_dist, 0); // context-free dist
        init();
    }

    ctxt_tree(param_t *p, rest *restaurant)
            : param(p), r(restaurant), latent(NULL) {
        init();
    }

    ctxt_tree(param_t *p, ctxt_tree *lat, rest *lam[])
            : param(p), latent(lat), lambda(lam) {
        rest **parents = new rest *[2];
        parents[0] = new rest(param, NULL, -1); // uniform
        parents[1] = latent->r;
        r = new rest(param, 0, parents, 2, lambda[0]);
        init();
    }

    ctxt_tree(param_t *p, rest *restaurant, ctxt_tree *lat, rest *lam[])
            : param(p), r(restaurant), latent(lat), lambda(lam) {
        init();
    }

    ctxt_tree *insert(word_t w) { // extend context to left by w
        if(children.count(w)) return children[w];

        // create new ctxt_tree
        int m = r->ctxt_len + 1;
        ctxt_tree *child;
        if(latent) {
            ctxt_tree *latent_child = latent->insert(w);
            rest **parents = new rest *[2];
            parents[0] = r; parents[1] = latent_child->r;
            rest *r_new = new rest(param, m, parents, 2, lambda[m]);
            child = new ctxt_tree(param, r_new, latent_child, lambda);
        } else {
            rest *r_new = new rest(param, r, m);
            child = new ctxt_tree(param, r_new);
        }
        children[w] = child;
        param->rest_count++;
        return child;
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

    void train(corpus &text, int start, int end) {
        for(int j = start; j < end; j++)
            insert_context(text, j)->add_cust(text[j]);
    }

    void resample_seat(void) { // reseat all customers
        r->resample();
        for(hash_map<word_t,ctxt_tree*>::iterator i = children.begin();
                i != children.end(); i++)
            i->second->resample();
    }

    void update_beta(void) { // update discount posterior
        r->update_beta();
        for(hash_map<word_t,ctxt_tree*>::iterator i = children.begin();
                i != children.end(); i++)
            i->second->update_beta(); // recur
    }

    void resample_param(void) {
        param->reset_beta();
        update_beta();
        param->resample_discount();
    }

    void resample(void) {
        resample_seat();
        resample_param();
    }
};
