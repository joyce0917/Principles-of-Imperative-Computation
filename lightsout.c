#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "lib/bitvector.h"
#include "lib/boardutil.h"
#include "lib/contracts.h"    
#include "lib/hdict.h"
#include "lib/heap.h"
#include "lib/queue.h"
#include "lib/xalloc.h"
#include "board-ht.h"

void Qfree(void *elem){
  free(elem);
}

void record(struct board_data* prev, bitvector B){
  if(!bitvector_equal(prev->board, B)){
	  record(prev->prev, B);
	  fprintf(stdout,"%d:%d\n",prev->row,prev->col);
  }else return;
}

bitvector press_button(bitvector B, int row, int col, uint8_t width, uint8_t height){
	uint8_t i = get_index(row,col,width,height);
	bitvector N = bitvector_flip(B,i);
	if(is_valid_pos(row, col-1, width, height)){
		i = get_index(row, col-1, width, height);
		N = bitvector_flip(N,i);
	}if(is_valid_pos(row, col+1, width, height)){
		i = get_index(row, col+1, width, height);
		N = bitvector_flip(N,i);
	}if(is_valid_pos(row-1, col, width, height)){
		i = get_index(row-1, col, width, height);
		N = bitvector_flip(N,i);
	}if(is_valid_pos(row+1, col, width, height)){
		i = get_index(row+1, col, width, height);
		N = bitvector_flip(N,i);
	}return N;
}


int search(bitvector B, uint8_t width, uint8_t height){
	struct board_data *first = xmalloc(sizeof(struct board_data));
	first->board = B;
	queue_t Q = queue_new();
	enq(Q,(queue_elem)first);
	hdict_t H = ht_new(1);
	while (!queue_empty(Q)){
		struct board_data *A = (struct board_data*)deq(Q);
		for (int row=0; row<height; row++){
			for (int col=0; col<width; col++){
				bitvector newboard = press_button(A->board,row,col,width,height);
				bitvector empty = bitvector_new();
				
				struct board_data *N = xmalloc(sizeof(struct board_data));
				N->board = newboard;
				N->prev = A;
				N->row=row;
				N->col=col;
				if (bitvector_equal(empty,newboard)){
					record(N,B);
					hdict_free(H);
					queue_free(Q, &Qfree);
					return 0;
				}if (ht_lookup(H,newboard)==NULL){
					ht_insert(H,N);
					enq(Q,N);
				}
			}
		}	
	}
	hdict_free(H);
	queue_free(Q,&Qfree);
	return 1;
}


int main(int argc, char **argv){
	if(argc !=2){
		fprintf(stderr, "Usage: lightsout <board name>\n");
		return 1;
	}
	char* name = argv[1];
	uint8_t* width = xmalloc(sizeof(uint8_t));
	uint8_t* height = xmalloc(sizeof(uint8_t));
	bitvector vector = bitvector_new();

	if(file_read(name, &vector,width, height)){
	    int result = search(vector,*width,*height);
	    free(width);
	    free(height);
	    if(result == 0){
	    	fprintf(stderr,"You got all the lights out!\n");
	    }else{
	    	fprintf(stderr,"No solution was found!\n");
	    }return result;
  	}fprintf(stderr,"Not Valid\n");
  	free(width);
	free(height);
	return 1;
}