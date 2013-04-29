#define NGRAM_SIZE 3 /* trigrams */
#define NDOM 2 /* general and specific domains */

#define BROWN_SIZE 1161192
#define SOU_TRAIN_SIZE 360634

void dhpylm(void) {
    corpus text("brown-sou");
    int vocab_size = text.vocab.size();
    int dom_start[] = {0, BROWN_SIZE, BROWN_SIZE + SOU_TRAIN_SIZE};

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

    int iters = 10;
    for(int i = 0; i < iters; i++) {
        printf("*** ITER %d ***\n", i);
        latent_root->resample();
        printf("LATENT: "); latent_param->print_discounts(); printf("\n");

        for(int j = 0; j < NDOM; j++) {
            printf("DOM%d: ", j);
            dom_root[j]->resample();
            dom_param[j]->print_discounts(); printf("\n");

            lambda_params[j]->reset_beta();
            for(int k = 0; k < NGRAM_SIZE; k++) {
                rest *r = lambdas[j][k];
                printf("  lambda_%d = %f (%d tabs, %d custs)\n",
                        k, r->p_word(0), r->ntab, r->ncust);
                r->update_beta();
            }
            lambda_params[j]->resample_discount();
            printf("  LAMBDA: ", j);
            lambda_params[j]->print_discounts(); printf("\n");
        }

        double bits = 0;
        for(int j = dom_start[NDOM]; j < text.size(); j++) {
            const rest *r = dom_root[NDOM-1]->get_context(text, j);
            double p = r->p_word(text[j]);
            bits += -log2(p);
        }
        printf("Perplexity: 2^%f = %f\n\n",
                bits/test_size, exp2(bits/test_size));
        fflush(stdout);
    }
}
