#include <stdlib.h>
#include <stdbool.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "queue.h"

typedef struct list_node list;
struct list_node {
  void *data;
  list *next;
};

typedef struct queue_header queue;
struct queue_header {
  list *front;
  list *back;
  size_t size;
};

bool is_inclusive_segment(list* start, list* end, size_t i) {
  if (i==0) return true;
  size_t a=0;
  for (list *p = start; p != NULL; p = p->next) {
    a=a+1;
    if (p == end && a==i) return true;
  }
  return false;
}

bool is_acyclic(list *start, list *end) {
for (list *p = start; p != end; p = p->next)
  { 
    if (p == NULL) return true; 
    for (list *q = start; q != p; q = q->next)
    { 
      if (q == p->next) return false; /* circular */ 
    } 
  } 
  return true; 
}


bool is_queue(queue* Q) {
  return Q != NULL && is_inclusive_segment(Q->front, Q->back, Q->size)
  && is_acyclic(Q->front,Q->back);
}

queue_t queue_new(){ 
  queue *Q = xmalloc(sizeof(queue));
  Q->front=NULL;
  Q->back=NULL;
  Q->size=0;

  return Q;
}

/* O(1) */
size_t queue_size(queue_t Q) 
{
  REQUIRES(is_queue(Q));
  REQUIRES(Q != NULL);

  return Q->size;
}

/* O(1) -- adds an item to the back of the queue */
void enq(queue_t Q, void* x) 
{
  REQUIRES(is_queue(Q));
  REQUIRES(Q!=NULL);
  list* l = xmalloc(sizeof(list));
  l->data=x;
  if (queue_size(Q)==0){ 
    Q->front = l;
    Q->back = l;
  }else{
  Q->back->next = l;
  Q->back = l;    
  }
  Q->size++;
  return;
}

/* O(1) -- removes an item from the front of the queue */
void* deq(queue_t Q)
{
  REQUIRES(is_queue(Q));
  REQUIRES(Q != NULL && queue_size(Q) > 0);
  void *s = Q->front->data;
  list *q=Q->front;
  Q->front = Q->front->next;
  Q->size--;
  Q->back->next=NULL;
  if (Q->size==0){
    Q->front=NULL;
    Q->back=NULL;
  }
  free(q);
  return s;
}

/* O(i) -- doesn't remove the item from the queue */
void *queue_peek(queue_t Q, size_t i)
{
  REQUIRES(is_queue(Q));
  REQUIRES(Q != NULL && 0 <= i && i < queue_size(Q));
  list *a=Q->front;
  for (int j=0; j<i; j++) {
    a=a->next;
  }
  return a->data;
}

/* O(n) */
void queue_reverse(queue_t Q)
  //@requires is_queue(Q); 
  /*@requires Q != NULL; @*/ 
  // @ensures is_queue(Q);
{
  if (Q->size==0) return;
  list* a=Q->back;
  list* c=NULL;
  while (Q->front!=a){
    list* b=Q->front;
    Q->front=Q->front->next;
    if (Q->back==a){
      a->next=b;
      Q->back=b;
      Q->back->next=NULL;
      c=b;
    }else{
      a->next=b;
      b->next=c; 
      c=b;
    }
  }
  return;
}


/* O(n) worst case, assuming P is O(1) */
bool queue_all(queue_t Q, check_property_fn* P)
  //@requires is_queue(Q); 
  /*@requires Q != NULL && P != NULL; @*/ 
{
  int size=queue_size(Q);
  for(int i=0; i<size; i++){
    if (!(*P)(queue_peek(Q,i))) return false;
  }return true;
}

/* O(n) worst case, assuming F is O(1) */
void* queue_iterate(queue_t Q, void* base, iterate_fn* F)
  //@requires is_queue(Q); 
  /*@requires Q != NULL && F != NULL; @*/ 
{
  int size=queue_size(Q);
  void* a= base;
  for (int i=0; i<size; i++){
    a=(*F)(a,queue_peek(Q,i));
  }return a;
}

void queue_free(queue_t Q, queue_elem_free_fn *F) {
  REQUIRES(is_queue(Q));
  while (Q->front != Q->back) {
    list *q = Q->front;
    if (F != NULL) (*F)(q->data);
    Q->front = Q->front->next;
    free(q);
  }
  free(Q->front);
  free(Q);
}
