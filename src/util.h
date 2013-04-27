#define EPS 1e-6
typedef int word_t;

double randu(double a, double b) { // U(a,b)
    return a + (b-a) * (double) rand() / RAND_MAX;
}

double randbeta(double a, double b) { // Beta(a,b)
    beta_distribution<> dist(a,b);
    return quantile(dist, randu(0,1)); // inverse transform
}

double uniform_dist(int size) {
    return 1.0 / size;
}

int uniform_sample(int size) {
    return rand() % size;
}

double log2_add(double log_a, double log_b) { // log(a),log(b) -> log(a+b)
    if(log_a < log_b) return log2_add(log_b, log_a); // ensure log_a >= log_b
    return log_a + log2(1 + exp2(log_b - log_a)); // log(a(1+b/a))
}

double log2_sub(double log_a, double log_b) { // log(a-b)
    return log_a + log2(1 - exp2(log_b - log_a)); // log(a(1-b/a))
}

double sample_simple(int N, double p[], double norm) {
    double U = randu(0, 1);
    for(int k = 0; k < N; k++) {
        U -= p[k] / norm;
        if(U < EPS) return k;
    }
    assert(0);
}
