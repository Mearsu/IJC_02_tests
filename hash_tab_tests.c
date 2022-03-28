//not needed, is removed when including, it's here just to make editor shut up
#ifndef UNUSED
#include <setjmp.h>
#include <stdio.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <string.h>
#include <stdlib.h>

#include "htab.h"
#include "htab_impl.h"
#define UNUSED(X) (void)(X)
#define tail_main(x, y) (void)(x); (void)(y)
#define OUT_REDIRECT "lol"
#endif

//macro to access i-th element from hash table. htab is private struct, so this
// might be different, arr_ptr in assignment 
//NOTE: tests expect htab struct to look like this, you can change following 
// macros if you have different names of fields
//struct htab{
//  size_t size;//number of htab_item
//  size_t arr_size;//size of arr[]
//  struct htab_item *arr[];
//};
#define GET_ARR_PTR(table, i) table->arr[i] 
// same but for size_t size
#define HTAB_SIZE(table) table->size 
// same but for size_t arr_size
#define HTAB_ARR_SIZE(table) table->arr_size 

void htab_init_test(void **state) {
  UNUSED(state);
  htab_t *tab = htab_init(10);
  for (int i = 0; i < 10; i++) {
    assert_int_equal(GET_ARR_PTR(tab, i), NULL);
  }
  assert_int_equal(HTAB_ARR_SIZE(tab), 10);
  htab_free(tab);
}

void htab_bucket_count_test(void** state){
  UNUSED(state);
  int count = htab_bucket_count(NULL);
  if(count != 0 && count != 01)
    fail_msg("htab_bucket_count should return 0 or -1 when passing NULL, not %d", count);
  htab_t *tab = htab_init(10);
  assert_int_equal(htab_bucket_count(tab), 10);
}

void test_hash_function_test(void ** state){
  UNUSED(state);  
//these tests might be wrond, change to 0 to disable
#if 1
  assert_int_equal(htab_hash_function("aa"), 6363200);
  assert_int_equal(htab_hash_function(""), 0);
  assert_int_equal(htab_hash_function(" "), 32);
#endif
}

void test_htab_size(void ** state){
  UNUSED(state);  
}

