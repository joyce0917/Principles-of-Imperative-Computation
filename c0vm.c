#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "lib/xalloc.h"
#include "lib/stack.h"
#include "lib/contracts.h"
#include "lib/c0v_stack.h"
#include "lib/c0vm.h"
#include "lib/c0vm_c0ffi.h"
#include "lib/c0vm_abort.h"

/* call stack frames */
typedef struct frame_info frame;
struct frame_info {
  c0v_stack_t S; /* Operand stack of C0 values */
  ubyte *P;      /* Function body */
  size_t pc;     /* Program counter */
  c0_value *V;   /* The local variables */
};

void elem_free(frame *F){
  c0v_stack_free(F->S);
  free(F->P);
  free(F->V);
  free(F);
}

int execute(struct bc0_file *bc0) {
  REQUIRES(bc0 != NULL);

  /* Variables */
  c0v_stack_t S = c0v_stack_new(); /* Operand stack of C0 values */
  ubyte *P = bc0->function_pool->code;      /* The array of bytes that make up the current function */
  size_t pc = 0;     /* Your current location within the current byte array P */
  c0_value *V = xcalloc(bc0->function_pool->num_vars, sizeof(struct c0_value));   /* The local variables (you won't need this till Task 2) */
  (void) V;

  /* The call stack, a generic stack that should contain pointers to frames */
  /* You won't need this until you implement functions. */
  gstack_t callStack = stack_new(); 
  (void) callStack;

  while (true) {
    
#ifdef DEBUG
    /* You can add extra debugging information here */
    // fprintf(stderr, "Opcode %x -- Stack size: %zu -- PC: %zu\n",
    //         P[pc], c0v_stack_size(S), pc);
#endif
    switch (P[pc]) {
      
    /* Additional stack operation: */

    case POP: {
      pc++;
      c0v_pop(S);
      break;
    }

    case DUP: {
      pc++;
      c0_value v = c0v_pop(S);
      c0v_push(S,v);
      c0v_push(S,v);
      break;
    }
      
    case SWAP: {
      pc++;
      c0_value v2 = c0v_pop(S);
      c0_value v1 = c0v_pop(S);
      c0v_push(S,v2);
      c0v_push(S,v1);   
      break;   
    }


    /* Returning from a function.
     * This currently has a memory leak! It will need to be revised
     * when you write INVOKESTATIC. */

    case RETURN: {
      c0_value retval = c0v_pop(S);
      assert(c0v_stack_empty(S));
#ifdef DEBUG
      if(retval.kind == C0_INTEGER){
        fprintf(stderr, "Returning %d from execute()\n", val2int(retval));
      } else if(retval.kind == C0_POINTER){
        fprintf(stderr, "Returning %c from execute()\n", *(char*)val2ptr(retval));
      }
#endif
      // Free everything before returning from the execute function!
      c0v_stack_free(S);
      free(V);
      if (stack_empty(callStack)) {
        stack_free(callStack,(stack_elem_free_fn*)&elem_free);
        if (retval.kind == C0_INTEGER) return val2int(retval);
        if (retval.kind == C0_POINTER) return *(char*)val2ptr(retval);
      }else{
        frame *F = pop(callStack);
        V=F->V;
        S=F->S;
        P=F->P;
        pc=F->pc;
        free(F);
        c0v_push(S,retval);
        break;
      }
    }



    /* Arithmetic and Logical operations */

    case IADD:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      c0v_push(S,int2val(x+y)); 
      break;   
    }
      
    case ISUB:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      c0v_push(S,int2val(x-y)); 
      break;  
    }
      
    case IMUL:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      c0v_push(S,int2val(x*y)); 
      break;  
    }
      
    case IDIV:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      if (y==0 || (y==-1 && x==(int32_t)0x80000000)){
        c0_arith_error("error");
      }
      c0v_push(S,int2val(x/y)); 
      break;     
    }
      
    case IREM:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      if (y==0 || (y==-1 && x==(int32_t)0x80000000)){
        c0_arith_error("error");
      }
      c0v_push(S,int2val(x%y)); 
      break;  
    }
      
    case IAND:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      c0v_push(S,int2val(x&y)); 
      break;    
    }
      
    case IOR:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      c0v_push(S,int2val(x|y)); 
      break;   
    }
      
    case IXOR:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      c0v_push(S,int2val(x^y)); 
      break;   
    }
      
    case ISHL:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      if (y>=32||y<0) c0_arith_error("error");
      c0v_push(S,int2val(x<<y)); 
      break;     
    }
      
    case ISHR:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      if (y>=32||y<0) c0_arith_error("error");
      c0v_push(S,int2val(x>>y)); 
      break;    
    }
      
      
    /* Pushing constants */

    case BIPUSH:{
      pc++;
      int32_t num = (int32_t)P[pc]; 
      num = (num<<24)>>24;
      c0v_push(S, int2val(num));
      pc++; 
      break;
    }

    case ILDC:{
      pc++;
      uint32_t c1 = P[pc];
      pc++;
      uint32_t c2 = P[pc];
      uint32_t i = (c1<<8)|c2;
      int32_t num = bc0->int_pool[i];
      c0v_push(S, int2val(num));
      pc++;
      break;
    }
      
    case ALDC:{
      pc++;
      uint32_t c1 = P[pc];
      pc++;
      uint32_t c2 = P[pc];
      uint32_t i = (c1<<8)|c2;
      void *str = &(bc0->string_pool)[i];
      c0v_push(S, ptr2val(str));
      pc++;
      break;
    }

    case ACONST_NULL:{
      pc++;
      c0v_push(S,ptr2val(NULL));
      break;
    }


    /* Operations on local variables */

    case VLOAD:{
      pc++;
      c0v_push(S,V[P[pc]]);
      pc++;
      break;
    }
      
    case VSTORE:{
      pc++;
      V[P[pc]]=c0v_pop(S);
      pc++;
      break;
    }
      
      
    /* Control flow operations */

    case NOP:{
      pc++;
      break;
    }
      
    case IF_CMPEQ:{
      int16_t i = (P[pc+1]<<8)|P[pc+2];
      c0_value y = c0v_pop(S);
      c0_value x = c0v_pop(S);
      if (val_equal(x,y)){
        pc=pc+i;
      }else{
        pc=pc+3;
      }
      break;
    }
      
    case IF_CMPNE:{
      int16_t i = (P[pc+1]<<8)|P[pc+2];
      c0_value y = c0v_pop(S);
      c0_value x = c0v_pop(S);
      if (!val_equal(x,y)){
        pc=pc+i;
      }else{
        pc=pc+3;
      }
      break;
    }
      
    case IF_ICMPLT:{
      int16_t i = (P[pc+1]<<8)|P[pc+2];
      c0_value y = c0v_pop(S);
      c0_value x = c0v_pop(S);
      if (val2int(x) < val2int(y)){
        pc=pc+i;
      }else{
        pc=pc+3;
      }
      break;
    }
      
    case IF_ICMPGE:{
      int16_t i = (P[pc+1]<<8)|P[pc+2];
      c0_value y = c0v_pop(S);
      c0_value x = c0v_pop(S);
      if (val2int(x) >= val2int(y)){
        pc=pc+i;
      }else{
        pc=pc+3;
      }
      break;
    }
      
    case IF_ICMPGT:{
      int16_t i = (P[pc+1]<<8)|P[pc+2];
      c0_value y = c0v_pop(S);
      c0_value x = c0v_pop(S);
      if (val2int(x) > val2int(y)){
        pc=pc+i;
      }else{
        pc=pc+3;
      }
      break;
    }
      
    case IF_ICMPLE:{
      int16_t i = (P[pc+1]<<8)|P[pc+2];
      c0_value y = c0v_pop(S);
      c0_value x = c0v_pop(S);
      if (val2int(x) <= val2int(y)){
        pc=pc+i;
      }else{
        pc=pc+3;
      }
      break;
    }
      
    case GOTO:{
      int16_t i = (P[pc+1]<<8)|P[pc+2];
      pc=pc+i;
      break;
    }
      
    case ATHROW:{
      pc++;
      c0_value a = c0v_pop(S);
      c0_user_error(val2ptr(a));
      break;
    }

    case ASSERT:{
      pc++;
      c0_value a = c0v_pop(S);
      c0_value x = c0v_pop(S);
      if(val2int(x)==0){
        c0_assertion_failure(val2ptr(a));
      }
      break;
    }


    /* Function call operations: */

    case INVOKESTATIC:{
      pc++;
      uint32_t c1 = P[pc];
      pc++;
      uint32_t c2 = P[pc];
      struct function_info fi = bc0->function_pool[(c1<<8)|c2];
      pc++;
      frame *F = xmalloc(sizeof(struct frame_info));
      F->V = V;
      F->P = P;
      F->S = S;
      F->pc = pc;
      push(callStack,(void*)F);
      V = xcalloc(fi.num_vars, sizeof(struct c0_value));
      for (size_t i=fi.num_args; i>0; i--){
        V[i-1] = c0v_pop(S);
      }
      S = c0v_stack_new();
      P = fi.code;
      pc=0;
      break;
    }
      

    case INVOKENATIVE:{
      pc++;
      uint32_t c1 = P[pc];
      pc++;
      uint32_t c2 = P[pc];
      struct native_info np = bc0->native_pool[(c1<<8)|c2];
      pc++;
      c0_value* n = xcalloc(np.num_args, sizeof(struct c0_value));
      for (size_t i=np.num_args; i>0; i--){
        n[i-1] = c0v_pop(S);
      }
      native_fn* f=native_function_table[np.function_table_index];
      c0v_push(S,(*f)(n));
      free(n);
      break;
    }


    /* Memory allocation operations: */

    case NEW:{
      pc++;
      void *n = (void*)xmalloc(P[pc]);
      c0v_push(S,ptr2val(n));
      pc++;
      break;
    }
      
    case NEWARRAY:{
      pc++;
      c0_array *n = xmalloc(sizeof(c0_array));
      int32_t c = val2int(c0v_pop(S));
      n->count = c;
      n->elt_size = P[pc];
      if (c==0){
        n->elems = NULL;
      }else{
        n->elems = xcalloc(n->count,n->elt_size);
      }
      c0v_push(S,ptr2val((void*)n));
      pc++;
      break;
    }
      
    case ARRAYLENGTH:{
      pc++;
      c0_array* A = val2ptr(c0v_pop(S));
      if (A == NULL){
        c0_arith_error("array length error");
      }else{
        c0v_push(S,int2val(A->count));
      }
      break;
    }


    /* Memory access operations: */

    case AADDF:{
      pc++;
      uint32_t f = P[pc];
      c0_array* a = val2ptr(c0v_pop(S));
      if (a==NULL){
        c0_arith_error("AADDF error");
      }else{
        c0v_push(S,ptr2val(a+f));
        pc++;
        break;
      }
    }
      
    case AADDS:{
      pc++;
      int32_t i = val2int(c0v_pop(S));
      c0_array* a = val2ptr(c0v_pop(S));
      if (a==NULL){
        c0_arith_error("AADDS error");
      }else if(0<=i && i<a->count){
        char* arr = (char*)a->elems+i*a->elt_size;
        c0v_push(S,ptr2val((void*)arr));
        break;
      }else{
        c0_memory_error("index error");
      }
    }
      
    case IMLOAD:{
      pc++;
      int32_t* a = (int32_t*)val2ptr(c0v_pop(S));
      if (a==NULL){
        c0_arith_error("imload error");
      }else{
        c0v_push(S,int2val(*a));
        break;
      }
    }
      
    case IMSTORE:{
      pc++;
      int32_t x = val2int(c0v_pop(S));
      int32_t* a = (int32_t*)val2ptr(c0v_pop(S));      
      if (a==NULL){
        c0_arith_error("imstore error");
      }else{
        *a=x;
        break;
      }
    }
      
    case AMLOAD:{
      pc++;
      void** a = (void**)val2ptr(c0v_pop(S));
      if (a==NULL){
        c0_arith_error("amload error");
      }else{
        c0v_push(S,ptr2val(*a));
        break;
      }
    }
      
    case AMSTORE:{
      pc++;
      void* b = val2ptr(c0v_pop(S));
      void** a = val2ptr(c0v_pop(S));
      if (a==NULL){
        c0_arith_error("amstore error");
      }else{
        *a=b;
        break;
      }
    }
      
    case CMLOAD:{
      pc++;
      char* a = (char*)val2ptr(c0v_pop(S));
      if (a==NULL){
        c0_arith_error("cmload error");
      }else{
        c0v_push(S,int2val(*a));
        break;
      }
    }
      
    case CMSTORE:{
      pc++;
      int32_t x = val2int(c0v_pop(S));
      char* a = (char*)val2ptr(c0v_pop(S));   
      if (a==NULL){
        c0_arith_error("cmstore error");
      }else{
        *a=(x&0x7f);
        break;
      }         
    }
      
    default:
      fprintf(stderr, "invalid opcode: 0x%02x\n", P[pc]);
      abort();
    }
  }
  
  /* cannot get here from infinite loop */
  assert(false);
}

