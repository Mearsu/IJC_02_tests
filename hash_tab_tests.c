// not needed, is removed when including, it's here just to make editor shut up
#ifndef UNUSED
#include <cmocka.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "htab.h"
#include "htab_impl.h"
#define UNUSED(X) (void)(X)
#define tail_main(x, y)                                                        \
  (void)(x);                                                                   \
  (void)(y)
#define OUT_REDIRECT "lol"
#endif

// macro to access i-th element from hash table. htab is private struct, so this
//  might be different, arr_ptr in assignment
// NOTE: tests expect htab struct to look like this, you can change following
//  macros if you have different names of fields
// struct htab{
//   size_t size;//number of htab_item
//   size_t arr_size;//size of arr[]
//   struct htab_item* (*arr)[];
// };
#define GET_ARR_PTR(table, i) (*table->arr)[i]
// same but for size_t size
#define HTAB_SIZE(table) table->size
// same but for size_t arr_size
#define HTAB_ARR_SIZE(table) table->arr_size

// one htab item
#define HTAB_ITEM struct htab_item
// how to access key in htab
#define HTAB_KEY(item) item->pair.key

static const char *g_htab_fails_msg;
// used to catch segfaults
static void htab_signal_catcher(int signo) {
  if (signo == SIGSEGV && g_htab_fails_msg != NULL)
    printf("Received segfault when testing hash table: %s\n", g_htab_fails_msg);
  exit(123);
}

int htab_test_teardown(void **state) {
  UNUSED(state);
  g_htab_fails_msg = NULL;
  return 0;
}

void htab_init_test(void **state) {
  UNUSED(state);
  htab_t *tab = htab_init(10);
  if (tab == NULL)
    return;
  for (int i = 0; i < 10; i++) {
    assert_int_equal(GET_ARR_PTR(tab, i), NULL);
  }
  assert_int_equal(HTAB_ARR_SIZE(tab), 10);
  htab_free(tab);
}

void htab_bucket_count_test(void **state) {
  UNUSED(state);
  g_htab_fails_msg = "running htab_bucket_count with NULL";
  int count = htab_bucket_count(NULL);
  if (count != 0 && count != 01)
    fail_msg(
        "htab_bucket_count should return 0 or -1 when passing NULL, not %d",
        count);
  htab_t *tab = htab_init(10);
  if (tab == NULL)
    return;
  assert_int_equal(htab_bucket_count(tab), 10);
}

void test_hash_function_test(void **state) {
  UNUSED(state);
// these tests might be wrond, change to 0 to disable
#if 1
  assert_int_equal(htab_hash_function("aa"), 6363200);
  assert_int_equal(htab_hash_function(""), 0);
  assert_int_equal(htab_hash_function(" "), 32);
#endif
}

void test_htab_insert(void **state) {
  UNUSED(state);
  signal(SIGSEGV, htab_signal_catcher);
  htab_t *tab = htab_init(1);
  if (tab == NULL)
    return;

  char str[2];
  // holds number of elements that were found after being inserted or failed to
  // insert (compensation for when malloc fails)
  int num_found = 0;
  for (int i = 0; i < 10; i++) {

    str[0] = 'a' + i;
    htab_pair_t *pair = htab_lookup_add(tab, str);
    if (pair)
      pair->value = i;
    else
      num_found++;
  }

  for (int i = 0; i < 10; i++) {
    str[0] = 'a' + i;
    htab_pair_t *pair = htab_find(tab, str);
    if (pair) {
      num_found++;
      pair->value = i;
      assert_int_equal(htab_find(tab, str)->value, i);
    }
  }
  // not every inserted element was found
  if (num_found != 10) {
    fail();
  }
}

void test_htab_find(void **state) {
  UNUSED(state);
  signal(SIGSEGV, htab_signal_catcher);
  htab_t *tab = htab_init(1);
  if (tab == NULL)
    return;
  char str[3];
  for (int i = 0; i < 2; i++)
    str[i] = 'a';
  str[2] = '\0';

  g_htab_fails_msg = "segfault when calling htab_lookup_add";
  htab_pair_t *pair = htab_lookup_add(tab, "aa");
  if (pair) {
    pair->value = 10;
    g_htab_fails_msg = "failed to retreive key from table";
    assert_ptr_not_equal(htab_find(tab, "aa"), NULL);
    // should fail when comparing strings using ==
    assert_ptr_not_equal(htab_find(tab, str), NULL);
  }
  g_htab_fails_msg = "segfault when calling htab_lookup_add";
  pair = htab_lookup_add(tab, "bb");
  if (pair) {
    pair->value = 10;
    g_htab_fails_msg = "failed to retreive key from table";
    assert_ptr_not_equal(htab_find(tab, "bb"), NULL);
  }

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

void htab_hash_function_test(void **state) {
  UNUSED(state); // cheeky touch of anarchism
  signal(SIGSEGV, htab_signal_catcher);
  g_htab_fails_msg = "running hash_function(NULL)";
  htab_hash_function(NULL);
}

void test_htab_resize(void **state) {
  UNUSED(state);
  signal(SIGSEGV, htab_signal_catcher);
  g_htab_fails_msg = "running htab_resize(NULL, num)";
  htab_resize(NULL, 0);

  g_htab_fails_msg = "running htab_resize(tab, 0)";
  htab_t *tab = htab_init(1);
  if (tab == NULL)
    return;
  htab_resize(tab, 0);
  g_htab_fails_msg = NULL;
}

void test_htab_erase(void **state) {
  UNUSED(state);
  signal(SIGSEGV, htab_signal_catcher);

  // add and remove 2 elements in same and reverse order
}

void empty_func(htab_pair_t *pair) {}

void tab_fe_help(htab_pair_t *pair) {
  if (strcmp("aa", pair->key) == 0)
    assert_int_equal(pair->value, 10);
  if (strcmp("bb", pair->key) == 0)
    assert_int_equal(pair->value, 20);
}

void test_htab_for_each(void **state) {
  UNUSED(state);
  signal(SIGSEGV, htab_signal_catcher);
  htab_t *tab = htab_init(2);
  if (tab == NULL)
    return;
  htab_pair_t *pair = htab_lookup_add(tab, "aa");
  if (pair)
    pair->value = 10;
  pair = htab_lookup_add(tab, "bb");
  if (pair)
    pair->value = 20;
  htab_for_each(tab, tab_fe_help);
  htab_for_each(tab, tab_fe_help);

  signal(SIGSEGV, htab_signal_catcher);
  g_htab_fails_msg = "running htab_for_each(NULL, func)";
  htab_for_each(NULL, empty_func);
  g_htab_fails_msg = "running htab_for_each(tab, NULL)";
  htab_for_each(tab, NULL);
  g_htab_fails_msg = NULL;

  htab_free(tab);
}

void test_htab_size(void **state) {
  UNUSED(state);
  signal(SIGSEGV, htab_signal_catcher);

  g_htab_fails_msg = "failed when calling htab_size(NULL)";
  htab_size(NULL);
}

void test_htab_sizes(void **state) {
  UNUSED(state);
  signal(SIGSEGV, htab_signal_catcher);
  htab_t *tab = htab_init(2);
  if (tab == NULL)
    return;

  int num_items = 0;

  assert_int_equal(htab_size(tab), num_items);
  htab_pair_t *pair = htab_lookup_add(tab, "aa");
  if (pair) {
    pair->value = 10;
    num_items++;
  }
  assert_int_equal(htab_size(tab), num_items);

  int bb_inserted = 0;
  pair = htab_lookup_add(tab, "bb");
  if (pair) {
    pair->value = 20;
    num_items++;
    bb_inserted = 1;
  }
  assert_int_equal(htab_size(tab), num_items);

  pair = htab_lookup_add(tab, "bb");
  if (pair && !bb_inserted) {
    pair->value = 20;
    num_items++;
  }

  assert_int_equal(htab_size(tab), num_items);
  htab_for_each(tab, tab_fe_help);
  htab_for_each(tab, tab_fe_help);
  htab_free(tab);
}

void test_htab_free(void **state) {
  UNUSED(state);
  signal(SIGSEGV, htab_signal_catcher);
  g_htab_fails_msg = "running htab_free(NULL)";
  htab_free(NULL);
  g_htab_fails_msg = NULL;
}
