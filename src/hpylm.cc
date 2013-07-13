#include <stdlib.h>

#include "util.h"
#include "HPYLM.h"

int main(int argc, char **argv) {
    assert(argc == 5);
    LOG("starting");
    HPYLM lm(atoi(argv[1]));
    R pplx = lm.run(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    LOG("final perplexity = %g", pplx);
    return 0;
}
