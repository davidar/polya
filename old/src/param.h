#define M 10

struct param_t {
    int N; // N-gram assumption
    double disc[M], conc[M]; // d_m, alpha_m
    double beta1[M], beta2[M]; // d_m ~ Beta(beta1[m], beta2[m])
    double gamma1[M], gamma2[M]; // alpha_m ~ Gamma(..)
    int vocab_size;
    int rest_count;

    param_t(int ngram_size, int nwords)
            : N(ngram_size), vocab_size(nwords), rest_count(0) {
        for(int i = 0; i < N; i++) {
            disc[i] = .5;
            conc[i] = 0;
        }
    }

    void reset_hyperparam(void) {
        for(int i = 0; i < N; i++)
            beta1[i] = beta2[i] = gamma1[i] = gamma2[i] = 1;
    }

    void resample(void) {
        for(int i = 0; i < N; i++) {
            disc[i] = randbeta(beta1[i], beta2[i]);
            conc[i] = randgamma(gamma1[i], gamma2[i]);
        }
    }

    void print(void) {
        for(int j = 0; j < N; j++)
            printf("d%d=%.2f; ", j, disc[j]);
        for(int j = 0; j < N; j++)
            printf("a%d=%.2f; ", j, conc[j]);
    }
};
