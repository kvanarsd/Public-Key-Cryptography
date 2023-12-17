#include "randstate.h"

#include <stdlib.h>

gmp_randstate_t state;

//initializes random usage
void randstate_init(uint64_t seed) {
    srandom(seed);
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, seed);
}

//clears all memory used by state
void randstate_clear() {
    gmp_randclear(state);
}
