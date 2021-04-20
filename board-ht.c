#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "lib/bitvector.h"
#include "lib/hdict.h"
#include "lib/contracts.h"
#include "lib/xalloc.h"


struct board_data {
  // Ignore this field (but don't remove it or change its type)
  uint8_t test;

  // This part of the struct acts like the key
  bitvector board;


  // You can add more fields to this struct
  // in order to help you implement the later tasks.
  struct board_data* prev;
  int row;
  int col;
};


//Knuth's method
size_t bit_hash(void *key){
	REQUIRES(key!=NULL);
	return (*((size_t*)key) * 2654435761) % (2^32);
}


bool bit_equal(void *one, void* two){
	REQUIRES(one!=NULL && two!=NULL);
	return bitvector_equal(*(bitvector*)one,*(bitvector*)two);
}

/* Initializes a new hash table with the given capacity */
hdict_t ht_new(size_t capacity){
	REQUIRES(capacity > 0);
	hdict_t result = hdict_new(capacity, &bit_equal, &bit_hash, NULL);
	ENSURES(result != NULL);
	return result;
}

/* ht_lookup(H,B) returns 
 * NULL if no struct containing the board B exists in H
 * A struct containing the board B if one exists in H.
 */
struct board_data *ht_lookup(hdict_t H, bitvector B){
	REQUIRES(H!=NULL);
	void* a = hdict_lookup(H,(void*)(&B));
	if(a== NULL) return NULL;
	return (struct board_data*) hdict_lookup(H,(void*)(&B));
}

/* ht_insert(H,e) has no return value, because it should have as
 * a precondition that no struct currently in the hashtable contains
 * the same board as DAT->board.
 */
void ht_insert(hdict_t H, struct board_data *DAT){
	REQUIRES(H != NULL || DAT != NULL);
	REQUIRES(ht_lookup(H, DAT->board) == NULL);

	// printf(DAT->board);
	hdict_insert(H, (void*)(&(DAT->board)), (void*)DAT);


	// if(hdict_lookup(H,(void*)(&(DAT->board))) != NULL)
	// 	printf("DUDE");
	// hdict_lookup(H,(void*)(&(DAT->board)));
	ENSURES(hdict_lookup(H,(void*)(&(DAT->board)))!= NULL);
}

















