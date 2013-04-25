// 2013 David A Roberts

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <vector>
#include <string>
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

int main(int argc, char **argv) {
#ifdef HPYLM
    assert(argc == 3);
    hpylm(atoi(argv[1]), atoi(argv[2]));
#endif
#ifdef PYPHMM
    pyphmm();
#endif
    return 0;
}
