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
//  struct htab_item* (*arr)[];
//};
#define GET_ARR_PTR(table, i) (*table->arr)[i] 
// same but for size_t size
#define HTAB_SIZE(table) table->size 
// same but for size_t arr_size
#define HTAB_ARR_SIZE(table) table->arr_size 

//one htab item
#define HTAB_ITEM struct htab_item
//how to access key in htab
#define HTAB_KEY(item) item->pair.key

static const char* g_htab_fails_msg;
//used to catch segfaults
static void htab_signal_catcher(int signo) {
  if (signo == SIGSEGV && g_htab_fails_msg != NULL)
      printf("Received segfault when testing hash table: %s\n",
         g_htab_fails_msg);
}

int htab_test_teardown(void ** state){
  UNUSED(state);
  g_htab_fails_msg = NULL;
  return 0;
}

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
  g_htab_fails_msg = "running htab_bucket_count with NULL";
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

void test_htab_insert(void** state){
  UNUSED(state);
  signal(SIGSEGV, htab_signal_catcher);
  htab_t *tab = htab_init(1);
  char str[2];
  for(int i = 0; i < 26; i++){
    str[0] = i + 'a';
    str[1] = '\0';
    htab_lookup_add(tab, str)->value = 10;
  }
  for(int i = 0; i < HTAB_ARR_SIZE(tab); i++){
    HTAB_ITEM *item = GET_ARR_PTR(tab, i);
    //my head hurts
    //TODO finish
  }
}

void test_htab_find(void ** state){
  UNUSED(state);  
  signal(SIGSEGV, htab_signal_catcher);
  htab_t *tab = htab_init(1);
  char str[3];
  for(int i = 0; i < 2; i++)
    str[i] = 'a';
  str[2] = '\0';

  g_htab_fails_msg = "segfault when calling htab_lookup_add";
  htab_lookup_add(tab, "aa")->value = 10;
  htab_lookup_add(tab, "bb")->value = 10;
  g_htab_fails_msg = "failed to retreive key from table";
  assert_ptr_not_equal(htab_find(tab, "aa"), NULL);
  assert_ptr_not_equal(htab_find(tab, "bb"), NULL);

  //should fail when comparing strings using ==
  assert_ptr_not_equal(htab_find(tab, str), NULL);

  g_htab_fails_msg = "failed accessing key that is not in table";
  assert_ptr_equal(htab_find(tab, "ab"), NULL);
  assert_ptr_equal(htab_find(tab, "a"), NULL);
  assert_ptr_equal(htab_find(tab, "aaa"), NULL);
  g_htab_fails_msg = "failed when calling htab_find(table, NULL)";
  assert_ptr_equal(htab_find(tab, NULL), NULL);
  g_htab_fails_msg = "failed when calling htab_find(NULL, NULL)";
  assert_ptr_equal(htab_find(NULL, NULL), NULL);
  g_htab_fails_msg = "failed when calling htab_find(NULL, str)";
  assert_ptr_equal(htab_find(NULL, "milk"), NULL);
}

void htab_hash_function_test(void ** state){
  UNUSED(state);// cheeky touch of anarchism
  signal(SIGSEGV, htab_signal_catcher);
  g_htab_fails_msg = "running hash_function(NULL)";
  htab_hash_function(NULL);
}

void test_htab_resize(void** state){
  UNUSED(state);  
  signal(SIGSEGV, htab_signal_catcher);
  g_htab_fails_msg = "running htab_resize(NULL, num)";
  htab_resize(NULL, 0);

  g_htab_fails_msg = "running htab_resize(tab, 0)";
  htab_t *tab = htab_init(1);
  htab_resize(tab, 0);
}


void test_htab_size(void ** state){
  UNUSED(state);  
  signal(SIGSEGV, htab_signal_catcher);

  g_htab_fails_msg = "failed when calling htab_size(NULL)";
  htab_size(NULL);
}

