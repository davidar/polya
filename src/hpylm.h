// undef to fix predictions during testing
//#define TEST_UPDATE

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
        param->print_discounts();

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
        double cdf_max = r->cdf(param->vocab_size - 1);
        if(abs(cdf_max - 1) > EPS)
            printf("WARNING cdf_max = %f for %dth word (%s)\n",
                    cdf_max, j, text.vocab[w].c_str());
    }
    printf("RANGE approx %.16f + 2^%f\n", low, log_range);
}
