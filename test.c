#define _POSIX_C_SOURCE 200809L
#include "stdio.h"
#undef _POSIX_C_SOURCE
#define main tail_main
#include "tail.c"
#undef main

// change to 0 to disable tests for tail/hash table
#define TEST_HTAB 1
#define TEST_TAIL 1

//can be set to number, which malloc should fail e.g. if it's set to 2, second
//malloc will fail
int MALLOC_NUM = -1;
#include "alloc_replace.c"

#if TEST_HTAB
//include all htab sources
#include "htab.h"
#include "htab_bucket_count.c"
#include "htab_clear.c"
#include "htab_erase.c"
#include "htab_find.c"
#include "htab_for_each.c"
#include "htab_free.c"
#include "htab_hash_function.c"
#include "htab_impl.h"
#include "htab_init.c"
#include "htab_lookup_add.c"
#include "htab_resize.c"
#include "htab_size.c"
#endif


#include "htab.h"
#include "htab_impl.h"

#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

//defines where to redirect stdout, stderr and stdin to again print to terminal
#if defined(_WIN32)
#warning "Not tested on windows, might not work"
#define OUT_REDIRECT "CON"
#elif defined(__linux__)
#define OUT_REDIRECT "/dev/tty"
#elif defined(__APPLE__)
#define OUT_REDIRECT "/dev/tty"
#endif

#define UNUSED(X) (void)(X)

#undef malloc
#undef free
#undef calloc
#undef realloc
#undef reallocarray
#undef fopen

#if TEST_HTAB
#include "hash_tab_tests.c"
#endif

#if TEST_TAIL
#include "tail_tests.c"
#endif

int main(int argc, char** argv) {
  if(argc > 1)
    MALLOC_NUM = atoi(argv[1]);

  #if TEST_HTAB
  const struct CMUnitTest htab_tests[] = {
      cmocka_unit_test(htab_init_test),
      cmocka_unit_test(htab_bucket_count_test),
      cmocka_unit_test(test_hash_function_test),
      cmocka_unit_test(test_htab_size),
      cmocka_unit_test(test_htab_insert),
      cmocka_unit_test(test_htab_find),
      cmocka_unit_test(test_hash_function_test),
      cmocka_unit_test(test_htab_resize),
      cmocka_unit_test(test_htab_for_each),
      cmocka_unit_test(test_htab_sizes),
      cmocka_unit_test(test_htab_free),
  };
  #endif

  #if TEST_TAIL
  const struct CMUnitTest tail_tests[] = {
      cmocka_unit_test(tail_single_file),
      cmocka_unit_test(tail_line_num_fail),
      cmocka_unit_test(tail_no_file),
      cmocka_unit_test(tail_no_file_len_arg),
      cmocka_unit_test(tail_test_n1),
      cmocka_unit_test(tail_test_n0),
      cmocka_unit_test(tail_test_multiple_files),
      cmocka_unit_test(tail_test_invalid_file),
      cmocka_unit_test(tail_test_multiple_ns),
      cmocka_unit_test(tail_test_dir),
  };
  #endif

  /* If setup and teardown functions are not
     needed, then NULL may be passed instead */
int failed = 0;
#if TEST_HTAB
  failed = cmocka_run_group_tests(htab_tests, NULL, htab_test_teardown);
#endif
#if TEST_TAIL
  failed += cmocka_run_group_tests(tail_tests, NULL, tail_test_teardown);
#endif

  return failed;
}

