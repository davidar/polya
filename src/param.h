struct param_t {
    int N; // N-gram assumption
    double discount[10]; // d_m
    double beta1[10], beta2[10]; // d_m ~ Beta(beta1[m], beta2[m])
    int vocab_size;
    int rest_count;

    param_t(int ngram_size, int nwords)
            : N(ngram_size), vocab_size(nwords), rest_count(0) {
        for(int i = 0; i < N; i++)
            discount[i] = .5;
    }

    void print_discounts(void) {
        for(int j = 0; j < N; j++)
            printf("d%d = %.3f; ", j, discount[j]);
    }
};
