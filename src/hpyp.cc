// 2013 David A Roberts

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <vector>
#include <string>
#include <algorithm>
using namespace std;

#define SPARSE_HASH 1
#if SPARSE_HASH
#include <sparsehash/sparse_hash_map>
using google::sparse_hash_map;
#define hash_map sparse_hash_map
#else
#include <sparsehash/dense_hash_map>
using google::dense_hash_map;
#define hash_map dense_hash_map
#endif

#include <boost/math/distributions.hpp>
using namespace boost::math;

#include "util.h"
#include "param.h"
#include "rest.h"
#include "corpus.h"
#include "ctxt_tree.h"

#include "hpylm.h"
#include "pyphmm.h"
#include "dhpylm.h"

int main(int argc, char **argv) {
#ifdef HPYLM
    assert(argc == 3);
    int ngram_size = atoi(argv[1]), test_size = atoi(argv[2]);

    // read data
    corpus text(stdin);
    int train_size = text.size() - test_size;
    printf("%d unique words, %d total\n",
        (int) text.vocab.size(), (int) text.size());
    printf("%d training, %d testing\n", train_size, test_size);
    fflush(stdout);

    hpylm(ngram_size, text, 0, train_size, test_size, 300, 125, true);
#endif
#ifdef PYPHMM
    pyphmm();
#endif
#ifdef DHPYLM
    assert(argc == 4);
    dhpylm(argv[1], argv[2], argv[3], 100);
#endif
    return 0;
}
