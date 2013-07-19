#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "HPYLM.h"
#include "SparseHPYLM.h"

inline LM *lm(N n) {
    const char *type = getenv("HPYLM");
    if(type == NULL) return new HPYLM(n);
    if(!strcmp(type, "sparse")) {
        if(n == 3) return new SparseHPYLM<2>();
        else if(n == 4) return new SparseHPYLM<3>();
        else assert(false);
    }
}

int main(int argc, char **argv) {
    assert(argc == 5);
    LOG("starting");
    R pplx = lm(atoi(argv[1]))->run(
            atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    LOG("final perplexity = %g", pplx);
    return 0;
}
