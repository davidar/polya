#define TAG_NGRAM 3 /* tag trigrams*/

void pyphmm(void) {
    // read data
    corpus words, true_tags;
    char buf1[100], buf2[100];
    while(scanf("%s %s", buf1, buf2) != EOF) {
        words.push_back(buf1);
        true_tags.push_back(buf2);
    }
    const int W = words.size(); // word count

    // init params, context trees
    const int T = 40; // number of available tag types
    param_t *t_param = new param_t(TAG_NGRAM, T);
    ctxt_tree *t_root = new ctxt_tree(t_param);
    param_t *w_param = new param_t(2, words.vocab.size());
    ctxt_tree *w_root = new ctxt_tree(w_param, new rest(w_param, NULL, 0));

    // init random tags
    corpus cur_tags;
    for(int j = 0; j < W; j++) {
        word_t t = uniform_sample(T), w = words[j];
        cur_tags.push_back(t);
        t_root->insert_context(cur_tags, j)->add_cust(t);
        w_root->insert(t)->r->add_cust(w);
    }
    printf("%d tag, %d word restaurants\n",
            t_param->rest_count, w_param->rest_count);

    // Gibbs sampling
    const int iters = 150;
    vector<vector<int> > tag_counts(W, vector<int>(T));
    for(int i = 0; i < iters; i++) {
        printf("ITER %3d: ", i);
        t_root->resample(); t_param->print_discounts();
        w_root->resample(); w_param->print_discounts();
        fflush(stdout);

        // resample tags
        double log_posterior = 0;
        for(int j = 0; j < W; j++) {
            const int ngram_end = min(j + TAG_NGRAM, W);
            word_t w = words[j];
            w_root->insert(cur_tags[j])->r->rm_cust(w); // rm emitted word
            for(int k = j; k < ngram_end; k++) // rm tag N-gram starting at j
                t_root->insert_context(cur_tags, k)->rm_cust(cur_tags[k]);

            // tag distribution
            double tag_probs[T], norm = 0;
            for(word_t t = 0; t < T; t++) {
                double p = w_root->get(t)->r->p_word(w);
                cur_tags[j] = t;
                for(int k = j; k < ngram_end; k++)
                    p *= t_root->get_context(cur_tags, k)->p_word(cur_tags[k]);
                tag_probs[t] = p; norm += p;
            }

            // sample new tag
            cur_tags[j] = sample_simple(T, tag_probs, norm);
            w_root->insert(cur_tags[j])->r->add_cust(w);
            for(int k = j; k < ngram_end; k++)
                t_root->insert_context(cur_tags, k)->add_cust(cur_tags[k]);

            log_posterior += log2(tag_probs[cur_tags[j]] / norm);
            tag_counts[j][cur_tags[j]]++;
        }
        printf("POST %.2f bits/word\n", -log_posterior/W);
    }
    printf("\n");

    // find most frequently sampled tag in each position
    // i.e. max marginal prob of tag wrt sample of tag seqs
    for(int j = 0; j < W; j++) {
        word_t t_best = 0;
        for(word_t t = 0; t < T; t++)
            if(tag_counts[j][t] > tag_counts[j][t_best])
                t_best = t;
        cur_tags[j] = t_best;
    }

    // tag summary
    for(word_t t = 0; t < T; t++) {
        printf("TAG id %d: ", t);

        vector<int> true_tag_freq(true_tags.vocab.size());
        int total_freq = 0;
        for(int j = 0; j < W; j++) {
            if(cur_tags[j] == t) {
                word_t t_true = true_tags[j];
                true_tag_freq[t_true]++;
                total_freq++;
            }
        }

        for(word_t t_true = 0; t_true < true_tags.vocab.size(); t_true++) {
            const char *tag_name = true_tags.name(t_true);
            double tag_freq = (double) true_tag_freq[t_true] / total_freq;
            if(tag_freq > .05)
                printf("%s: %.1f%%; ", tag_name, 100 * tag_freq);
        }
        printf("\n");

        const rest *w_r = w_root->get(t)->r;
        printf("e.g. ");
        for(int i = 0; i < 30; i++)
            printf("%s, ", words.name(w_r->sample_word()));
        printf("...\n\n");
    }

    // sample
    corpus tag_sample;
    int sample_size = 250;
    printf("SAMPLE TEXT\n... ");
    for(int j = 0; j < sample_size; j++) {
        word_t t = t_root->get_context(tag_sample, j)->sample_word();
        word_t w = w_root->get(t)->r->sample_word();
        printf("%s ", words.name(w));
        tag_sample.push_back(t);
    }
    printf("...\n");
}
