#define main tail_main
#include "tail.c"
#undef main

#include "htab.h"
#include "htab_impl.h"

#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>

// change to 0 to disable tests for tail/hash table
#define TEST_HTAB 1
#define TEST_TAIL 1

//defines where to redirect stdout, stderr and stdin to again print to terminal
#if defined(_WIN32)
#warning "Not tested on windows, might not work"
#define OUT_REDIRECT "CON"
#elif defined(__linux__)
#define OUT_REDIRECT "/dev/tty"
#elif defined(__APPLE__)
#define OUT_REDIRECT "/dev/tty"
#endif

#include <cmocka.h>

#define UNUSED(X) (void)(X)

//cond - check condition e.g. == 0 would check that nothing was read from stderr file
//msg - message to print when condition is met
#define CHECK_STDERR(cond, msg)                                                \
  do {                                                                         \
    FILE *serr = fopen("stderr", "r");                                         \
    char serr_buff[256];                                                       \
    int serr_read_size = fread(serr_buff, 1, sizeof(serr_buff), serr);         \
    if (serr_read_size cond)                                                   \
      fail_msg(msg);                                                           \
    fclose(serr);                                                              \
  } while (0)

#define CHECK_STDOUT(cond, msg)                                                \
  do {                                                                         \
    FILE *sout = fopen("stdout", "r");                                         \
    char sout_buff[256];                                                       \
    int sout_read_size = fread(sout_buff, 1, sizeof(sout_buff), sout);         \
    if (sout_read_size cond)                                                   \
      fail_msg(msg);                                                           \
    fclose(sout);                                                              \
  } while (0)

void create_file(const char* name, int num_lines){
  FILE* f = fopen(name, "w");
  if(f == NULL){
    fail_msg("Failed to open file \"infile\"");
  }
  for(int i = 0; i < num_lines; i++){
    fprintf(f, "Line%d\n", i);
  }
  fclose(f);
}

// used to report which argument combination caused segfault
static char *g_tail_current_args = NULL;

void tail_teardown();
//used to catch segfault from tail
static void signal_catcher(int signo) {
  tail_teardown();
  if (signo == SIGSEGV && g_tail_current_args != NULL)
      printf("Received segfault when running tail with \"%s\"\n",
         g_tail_current_args);
}

// checks if file stdout contains x lines containing "Line<num>"
// num - number of lines
// out_offset - files are expected to be generated using create_file out_offset
// is how many lines were dropped by tail e.g. when raeding file with 15 lines
// using tail with no arguments, offset should be 5 because only last 10 lines
// are outputted
void check_stdout_lines(int num, int out_offset) {
  FILE *sout = fopen("stdout", "r");
  char *line_buff = NULL;
  size_t line_len = 0;
  char test_buff[20];
  for(int i = 0; i < num; i++){
    if(getline(&line_buff, &line_len, sout) == -1)
      fail_msg("tail outputed only %d lines when running with %s", i, g_tail_current_args);
    sprintf(test_buff, "Line%d\n", i +  out_offset);
    if(strcmp(line_buff, test_buff) != 0)
      fail_msg("Line %d outputted by tail contains \"%s\", expected %s\n running with %s", i, line_buff, test_buff, g_tail_current_args);
  }
  if(getline(&line_buff, &line_len, sout) != -1)
      fail_msg("tail outputed more than %d line when running with %s", num, g_tail_current_args);
  free(line_buff);
  fclose(sout);
}

void htab_init_test(void **state) {
  UNUSED(state);
  htab_t *tab = htab_init(10);
  for (int i = 0; i < 10; i++) {
    assert_int_equal(tab->arr[i], NULL);
  }
  assert_int_equal(htab_bucket_count(tab), 10);
  assert_int_equal(tab->arr_size, 10);
  htab_free(tab);
}

// note these cannot be test setup/teardowns, it would redirect test results as
// well redirects stdout and stderr to files called "stdout" and "stderr" in
// same dir stdin_new - where to redirect stdin
void tail_setup(const char *stdin_new) {
  freopen("stdout", "w", stdout);
  freopen("stderr", "w", stderr);
  freopen(stdin_new, "r", stdin);
}

// restores stdin and stderr
void tail_teardown() {
  freopen(OUT_REDIRECT, "w", stdout);
  freopen(OUT_REDIRECT, "w", stderr);
  freopen(OUT_REDIRECT, "r", stdin);
}



void tail_line_num_fail(void **state) {
  signal(SIGSEGV, signal_catcher);//to catch segfault
  UNUSED(state);
  char *args[7][4] = {//different argument combinations
  {"./tail", "-n", "10a"},
  {"./tail", "-n", "a10"},
  {"./tail", "-n", "a"},
  {"./tail", "-n"},
  {"./tail", "-n10a"},
  {"./tail", "-na"},
  {"./tail", "-n"},
  };
  g_tail_current_args = malloc(256);

  for (int i = 0; i < sizeof(args) / sizeof(args)[0]; i++) {
    strcpy(g_tail_current_args, args[i][1]);//copy second argument for signal_catcher
    if (args[i][2] != NULL) {//copy third argument if exists
      strcat(g_tail_current_args, " ");
      strcat(g_tail_current_args, args[i][2]);
    }
    //count arguments
    int arg_num = 0;
    while (args[i][arg_num++] != NULL);

    tail_setup("/dev/null");
    tail_main(arg_num, args[i]);
    tail_teardown();
    CHECK_STDERR(== 0, "Tail did not report error when passing \"%s\" as arguments");
    CHECK_STDOUT(!= 0, "Tail did output to stdout when passing \"%s\" as arguments, should only report error when passing invalid arguments");

  }
  free(g_tail_current_args);
  g_tail_current_args = NULL;
  remove("stderr");
  remove("stdout");
}

void tail_single_file(void **state) {
  signal(SIGSEGV, signal_catcher); // to catch segfault
  UNUSED(state);
  // generate input file
  create_file("infile", 11);

  char *args[3] = {"./tail", "infile"};  //< arguments for tail
  g_tail_current_args = "./tail infile"; // arguements for catching segfault
  tail_setup("/dev/null");
  tail_main(2, args);
  tail_teardown();

  CHECK_STDERR(!= 0, "tail outputted error when passed file \"infile\" as argument");

  g_tail_current_args = NULL;
  remove("stderr");
  remove("stdout");
  remove("infile");
}


void tail_no_file(void** state){
  signal(SIGSEGV, signal_catcher);//to catch segfault
  UNUSED(state);
  create_file("infile", 15);

  char *args[] = {"./tail", NULL};  //< arguments for tail
  g_tail_current_args = "./tail";
  tail_setup("infile");
  tail_main(sizeof(args) / sizeof(args[0]), args);
  tail_teardown();

  CHECK_STDERR(!= 0, "Tail should not output to stderr, when passing no arguments");

  check_stdout_lines(10, 5);

  g_tail_current_args = NULL;
  remove("infile");
  remove("stdout");
  remove("stderr");
}


void tail_no_file_len_arg(void** state){
  signal(SIGSEGV, signal_catcher);//to catch segfault
  UNUSED(state);
  create_file("infile", 11);

  char *args[] = {"./tail", "-n3", NULL};  //< arguments for tail
  g_tail_current_args = "./tail -n3";
  tail_setup("infile");
  tail_main(sizeof(args) / sizeof(args[0]), args);
  tail_teardown();

  CHECK_STDERR(!= 0, "Tail should not output to stderr, when receiving \"-n3\" as arguments");
  check_stdout_lines(3, 8);

  g_tail_current_args = NULL;
  remove("infile");
  remove("stdout");
  remove("stderr");
}

void tail_test_multiple_files(void** state){
  UNUSED(state);
}

int main(void) {
  #if TEST_HTAB
  const struct CMUnitTest htab_tests[] = {
      cmocka_unit_test(htab_init_test),
  };
  #endif

  #if TEST_TAIL
  const struct CMUnitTest tail_tests[] = {
      cmocka_unit_test(tail_single_file),
      cmocka_unit_test(tail_line_num_fail),
      cmocka_unit_test(tail_no_file),
      cmocka_unit_test(tail_no_file_len_arg),
  };
  #endif

  /* If setup and teardown functions are not
     needed, then NULL may be passed instead */
int failed = 0;
#if TEST_HTAB
  failed = cmocka_run_group_tests(htab_tests, NULL, NULL);
#endif
#if TEST_TAIL
  failed += cmocka_run_group_tests(tail_tests, NULL, NULL);
#endif

  return failed;
}
