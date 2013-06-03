#define NGRAM_SIZE 3 /* trigrams */
#define NDOM 2 /* general and specific domains */

void dhpylm(const char *dom0_fname, const char *dom1_train_fname,
        const char *dom1_test_fname, int iters) {
    corpus text;
    int dom0_size = text.read(dom0_fname);
    int dom1_train_size = text.read(dom1_train_fname);
    int dom1_test_size = text.read(dom1_test_fname);
    int vocab_size = text.vocab.size();
    int dom_start[] = {0, dom0_size, dom0_size + dom1_train_size};
    printf("%d dom0, %d dom1 train, %d dom1 test\n",
            dom0_size, dom1_train_size, dom1_test_size);
    printf("%d unique words\n", vocab_size);

    printf("\n*** BASELINE hpylm ***\n");
    hpylm(NGRAM_SIZE, text,
            dom0_size, dom1_train_size, dom1_test_size,
            iters, 0, false);
    printf("\n*** UNION hpylm ***\n");
    hpylm(NGRAM_SIZE, text,
            0, dom0_size + dom1_train_size, dom1_test_size,
            iters, 0, false);
    printf("\n*** DHPYLM ***\n");

    param_t *latent_param = new param_t(NGRAM_SIZE, vocab_size);
    ctxt_tree *latent_root = new ctxt_tree(latent_param);

    param_t **lambda_params = new param_t *[NDOM];
    rest ***lambdas = new rest **[NDOM];
    param_t **dom_param = new param_t *[NDOM];
    ctxt_tree **dom_root = new ctxt_tree *[NDOM];
    for(int j = 0; j < NDOM; j++) {
        // per-domain lambda
        lambda_params[j] = new param_t(NGRAM_SIZE, 2);
        lambdas[j] = new rest *[NGRAM_SIZE];
        rest *unif2 = new rest(lambda_params[j], NULL, -1);
        for(int k = 0; k < NGRAM_SIZE; k++)
            lambdas[j][k] = new rest(lambda_params[j], unif2, k);

        dom_param[j] = new param_t(NGRAM_SIZE, vocab_size);
        dom_root[j] = new ctxt_tree(dom_param[j], latent_root, lambdas[j]);

        // train
        int start = dom_start[j], end = dom_start[j+1];
        printf("TRAIN dom%d: ", j); text.print(start, 100);
        dom_root[j]->train(text, start, end);
    }
    int test_size = text.size() - dom_start[NDOM];

    printf("CREATED %d latent, %d dom0, %d dom1 restaurants\n",
            latent_param->rest_count, dom_param[0]->rest_count,
            dom_param[1]->rest_count);

    printf("TEST dom%d: ", NDOM-1);
    text.print(dom_start[NDOM], 100); printf("\n");
    fflush(stdout);

    double log_sum_prob = log2(0);
    for(int i = 0; i < iters; i++) {
        printf("ITER %3d: ", i);
        latent_root->resample();

        for(int j = 0; j < NDOM; j++) {
            dom_root[j]->resample();

            lambda_params[j]->reset_hyperparam();
            for(int k = 0; k < NGRAM_SIZE; k++) {
                rest *r = lambdas[j][k];
                printf("l%d=%.3f; ", k, r->p_word(0));
                r->update_hyperparam();
            }
            lambda_params[j]->resample();
        }

        double bits = 0;
        for(int j = dom_start[NDOM]; j < text.size(); j++) {
            const rest *r = dom_root[NDOM-1]->get_context(text, j);
            double p = r->p_word(text[j]);
            bits += -log2(p);
        }
        log_sum_prob = log2_add(log_sum_prob, -bits);
        printf("PPLX 2^%.2f = %.1f\n",
                bits/test_size, exp2(bits/test_size));
        fflush(stdout);
    }

    printf("\nSUMMARY\n");
    double log_mean = log_sum_prob - log2(iters);
    printf("Perplexity: 2^%f = %f\n",
            -log_mean / test_size, exp2(-log_mean / test_size));
}
