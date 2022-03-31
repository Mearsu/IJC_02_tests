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

#define UNUSED(X) (void)(X)
#define tail_main(x, y) (void)(x); (void)(y)
#define OUT_REDIRECT "lol"
#endif

// used to report which argument combination caused segfault
static char *g_tail_current_args = NULL;

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

//name -filename to create-iin
//num_lines - how many lines to create
//offset - from which line start counting
void create_file(const char* name, int num_lines, int offset){
  FILE* f = fopen(name, "w");
  if(f == NULL){
    fail_msg("Failed to open file \"infile\"");
  }
  for(int i = offset; i < num_lines + offset; i++){
    fprintf(f, "%cLine%d\n",255, i);
  }
  fclose(f);
}


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
#define check_stdout_lines(num, out_offset)                                    \
  do {                                                                         \
    FILE *sout = fopen("stdout", "r");                                         \
    char *line_buff = NULL;                                                    \
    size_t line_len = 0;                                                       \
    char test_buff[20];                                                        \
    for (int i = 0; i < num; i++) {                                            \
      if (getline(&line_buff, &line_len, sout) == -1)                          \
        fail_msg("tail outputed only %d lines when running with %s\nNOTE: lines contain 255 bit at beginning\n", i,        \
                 g_tail_current_args);                                         \
      sprintf(test_buff, "%cLine%d\n",255, i + out_offset);                          \
      if (strcmp(line_buff, test_buff) != 0)                                   \
        fail_msg("Line %d outputted by tail contains \"%s\", expected %s\n NOTE: lines contain 255 bit at beginning\n"   \
                 "running with %s",                                            \
                 i, line_buff, test_buff, g_tail_current_args);                \
    }                                                                          \
    if (getline(&line_buff, &line_len, sout) != -1)                            \
      fail_msg("tail outputed more than %d line when running with %s", num,    \
               g_tail_current_args);                                           \
    free(line_buff);                                                           \
    fclose(sout);                                                              \
  } while (0)
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

// Testing invalid line numbers, should fail in all cases
void tail_line_num_fail(void **state) {
  signal(SIGSEGV, signal_catcher);//to catch segfault
  UNUSED(state);
  create_file("infile", 1, 0);
  char *args[9][4] = {//different argument combinations
  {"./tail", "-n", "10a"},
  {"./tail", "-n", "a10"},
  {"./tail", "-n", "a"},
  {"./tail", "-n"},
  {"./tail", "-n10a"},
  {"./tail", "-na"},
  {"./tail", "-n"},
  {"./tail", "-n", "infile"},
  {"./tail", "-n", "hope_this_file_doesnt_exist"},
  };
  g_tail_current_args = malloc(256);

  for (size_t i = 0; i < sizeof(args) / sizeof(args)[0]; i++) {
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
    CHECK_STDERR(== 0, "Tail did not report error when passing invalid line as argument");
    CHECK_STDOUT(!= 0, "Tail did output to stdout when passing invalid line as arguments, should only report error when passing invalid arguments");

  }
  free(g_tail_current_args);
}

//testing output when running on single file with default number (10)
//expects last 10 lines from infile to be outputted to stdout, and stderr to remain empty
void tail_single_file(void **state) {
  signal(SIGSEGV, signal_catcher); // to catch segfault
  UNUSED(state);
  // generate input file
  create_file("infile", 11, 0);

  char *args[3] = {"./tail", "infile"};  //< arguments for tail
  g_tail_current_args = "./tail infile"; // arguements for catching segfault
  tail_setup("/dev/null");
  tail_main(2, args);
  tail_teardown();

  CHECK_STDERR(!= 0, "tail outputted error when passed file \"infile\" as argument");

}

// testing reading from stdin, same as with single file, should read from stdin
void tail_no_file(void** state){
  signal(SIGSEGV, signal_catcher);//to catch segfault
  UNUSED(state);
  create_file("infile", 15, 0);

  char *args[] = {"./tail", NULL};  //< arguments for tail
  g_tail_current_args = "./tail";
  tail_setup("infile");
  tail_main(sizeof(args) / sizeof(args[0]), args);
  tail_teardown();

  CHECK_STDERR(!= 0, "Tail should not output to stderr, when passing no arguments");

  check_stdout_lines(10, 5);

}


// expect last x lines from stdin to apear on stdout, stderr to remain empty
void tail_no_file_len_arg(void** state){
  signal(SIGSEGV, signal_catcher);//to catch segfault
  UNUSED(state);
  create_file("infile", 11, 0);

  char *args[] = {"./tail", "-n3", NULL};  //< arguments for tail
  g_tail_current_args = "./tail -n3";
  tail_setup("infile");
  tail_main(sizeof(args) / sizeof(args[0]), args);
  tail_teardown();

  CHECK_STDERR(!= 0, "Tail should not output to stderr, when receiving \"-n3\" as arguments");
  check_stdout_lines(3, 8);

}

//test output with -n 1 might cause problems to some implementations
void tail_test_n1(void** state){
  signal(SIGSEGV, signal_catcher);
  UNUSED(state);
  create_file("infile", 5, 0);
  char* args[] = {"./tail", "-n", "1", "infile", NULL};
  g_tail_current_args = "./tail -n 1 infile";
  tail_setup("/dev/null");
  tail_main(sizeof(args) / sizeof(args[0]), args);
  tail_teardown();
  CHECK_STDERR(!= 0, "Tail should not output anything to stderr when running with \"./tail -n 1 ...\"");
  
  check_stdout_lines(1, 4);
}

//testing with -0 as argument, should not output anything
void tail_test_n0(void** state){
  signal(SIGSEGV, signal_catcher);//to catch segfault
  UNUSED(state);
  create_file("infile", 5, 0);
  create_file("infile1", 5, 0);

  {
  char *args[] = {"./tail", "-n0", "infile", NULL};  //< arguments for tail
  g_tail_current_args = "./tail -n0 infile";
  tail_setup("/dev/null");
  tail_main(sizeof(args) / sizeof(args[0]), args);
  tail_teardown();
  CHECK_STDERR(!= 0, "Tail should not output anything to stderr when running with \"./tail -n0 ...\"");
  CHECK_STDOUT(!= 0, "Tail should not output anything to stdout when running with \"./tail -n0 ...\"");

  char *args_[] = {"./tail", "-n", "0", "infile", NULL};  //< arguments for tail
  g_tail_current_args = "./tail -n 0 infile";
  tail_setup("/dev/null");
  tail_main(sizeof(args_) / sizeof(args_[0]), args_);
  tail_teardown();
  CHECK_STDERR(!= 0, "Tail should not output anything to stderr when running with \"./tail -n 0 ...\"");
  CHECK_STDOUT(!= 0, "Tail should not output anything to stdout when running with \"./tail -n 0 ...\"");
  }

  //testing if tail outputs nothing when passing multiple files with -n0
  char *args[] = {"./tail", "-n", "0", "infile", "infile1", NULL};  //< arguments for tail
  g_tail_current_args = "./tail -n0 infile infile1";
  tail_setup("/dev/null");
  tail_main(sizeof(args) / sizeof(args[0]), args);
  tail_teardown();
  CHECK_STDERR(!= 0, "Tail should not output anything to stderr when running with \"./tail -n0 ...\"");
  CHECK_STDOUT(!= 0, "Tail should not output anything to stdout when running with \"./tail -n0 ...\"");
}

void tail_test_multiple_files(void** state){
  UNUSED(state);
  create_file("infile1", 15, 0);
  create_file("infile2", 15, 10);

  char *args[] = {"./tail", "infile1", "infile2", NULL};  //< arguments for tail
  g_tail_current_args = "./tail infile1 infile2";
  tail_setup("/dev/null");
  tail_main(sizeof(args) / sizeof(args[0]), args);
  tail_teardown();
  CHECK_STDERR(!= 0, "Tail should not output anything to stderr when running with \"./tail infile1 infile2\"");

  FILE *sout = fopen("stdout", "r");
  char *line_buff = NULL;
  size_t line_len = 0;
  char test_buff[20];

  const int help_line = __LINE__;
  //============================================================================
  //check output when passing multiple files to tail
  //expected format is
  //line identifiing file e.g. ==> infile1 <==
  //output of first file (10 lines)
  //line identifiing file e.g. ==> infile2 <==
  //output of second file (10 lines)
  //============================================================================
  //first file identifier, e.g. ==> infile1 <==
  if (getline(&line_buff, &line_len, sout) == -1)
    fail_msg("Tail didn't output anything when using './tail infile1 infile2'");

//check if tail outputted end of first file
  for (int i = 0; i < 10; i++) {
    if (getline(&line_buff, &line_len, sout) == -1)
      fail_msg("tail outputed only %d lines when running with %s", i,
               g_tail_current_args);
    sprintf(test_buff, "%cLine%d\n",255, i + 5);
    if (strcmp(line_buff, test_buff) != 0)
      fail_msg("Line %d outputted by tail contains \"%s\", expected %s\n "
               "running with %s",
               i, line_buff, test_buff, g_tail_current_args);
  }

  //second file identifier, e.g. ==> infile2 <==
  if (getline(&line_buff, &line_len, sout) == -1)
    fail_msg("Taill outputted only one file when passed multiple files");

//check if tail outputted end of second file
  for (int i = 0; i < 10; i++) {
    if (getline(&line_buff, &line_len, sout) == -1)
      fail_msg("tail outputed only %d lines when running with %s, are you including file identifiers? see %s:%d", i,
               g_tail_current_args, __FILE__, help_line);
    sprintf(test_buff, "%cLine%d\n",255, i + 5 + 10);
    if (strcmp(line_buff, test_buff) != 0)
      fail_msg("Line %d outputted by tail contains \"%s\", expected %s\n "
               "running with %s",
               i, line_buff, test_buff, g_tail_current_args);
  }
  if (getline(&line_buff, &line_len, sout) != -1)
    fail_msg("Tail outputted more than 2x10 lines from files + 2 file identifiers, see %s:%d", __FILE__, help_line);

  free(line_buff);
  fclose(sout);
}

//checking that tail outputs error when file doesn't exist
void tail_test_invalid_file(void** state){
  UNUSED(state);//no i'm not anarchist

  remove("infile");//tests would override infile anyway :)
  char *args[] = {"./tail", "infile", NULL};  //< arguments for tail
  g_tail_current_args = "./tail <non-existing file>";
  tail_setup("/dev/null");
  tail_main(sizeof(args) / sizeof(args[0]), args);
  tail_teardown();
  CHECK_STDERR(== 0, "tail did not output to stderr when running on non-existing file");
  CHECK_STDOUT(!= 0, "Tail should not output anything to stdout when running on non-existing file");

}

//tests what happens when multiple -n arguments are passed, last one should be used
void tail_test_multiple_ns(void** state){
  signal(SIGSEGV, signal_catcher);
  UNUSED(state);
  create_file("infile", 10, 0);
  char* args[] = {"./tail", "-n", "5", "-n2", "infile", NULL};
  g_tail_current_args = "./tail -n 5 -n2 infile";
  tail_setup("/dev/null");
  tail_main(sizeof(args) / sizeof(args[0]), args);
  tail_teardown();
  CHECK_STDERR(!= 0, "Tail should not output anything to stderr when running with \"./tail -n 1 ...\"");
  
  check_stdout_lines(2, 8);
}

// try running tail on directory
void tail_test_dir(void ** state){
  signal(SIGSEGV, signal_catcher);
  UNUSED(state);
  char* args[] = {"./tail", ".", NULL};
  g_tail_current_args = "./tail .";
  tail_setup("/dev/null");
  tail_main(sizeof(args) / sizeof(args[0]), args);
  tail_teardown();
  CHECK_STDERR(== 0, "tail did not output to stderr when running on directory");
  CHECK_STDOUT(!= 0, "Tail should not output anything to stdout when running on directory");
}

int tail_test_teardown(void ** state){
  UNUSED(state);
  g_tail_current_args = NULL;
  remove("infile");
  remove("infile1");
  remove("infile2");
  remove("stdout");
  remove("stderr");
  return 0;
}

