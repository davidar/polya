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
