#include <stdbool.h>
#include <stdlib.h>
#include "lib/bitvector.h"
#include "lib/contracts.h"
#include "lib/xalloc.h"

/* Get a new bitvector with everything set to 'false'. */
bitvector bitvector_new(){
	return (bitvector) 0;
}

/* Get the ith bit of the bitvector n. */
bool bitvector_get(bitvector B, uint8_t i){
	REQUIRES(i < BITVECTOR_LIMIT && BITVECTOR_LIMIT<= 8*sizeof(B));
	return B>>i & 1;
}

/* Toggle the ith bit of the bitvector n, returning a new bitvector. */
/* The old bitvector remains unchanged. */
bitvector bitvector_flip(bitvector B, uint8_t i){
	REQUIRES(i < BITVECTOR_LIMIT && BITVECTOR_LIMIT<= 8*sizeof(B));
	return B^(1<<i);
}

/* Compare two bitvectors for equality. */
bool bitvector_equal(bitvector B1, bitvector B2){
	REQUIRES(BITVECTOR_LIMIT <= 8*sizeof(B1));
	REQUIRES(BITVECTOR_LIMIT <= 8*sizeof(B2));
	return B1==B2;
}
