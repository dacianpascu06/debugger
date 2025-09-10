#ifndef UTILS_H
#define UTILS_H

#include "../elf_parser/elf_parser.h"
#include "breakpoint_queue.h"
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
  pid_t target_pid;
  ElfFile *elf_file;
  uint64_t base_addr;
  BreakpointQueue bp_queue;
  siginfo_t sig_info;
  struct user_regs_struct regs;
} GlobalContext;

typedef struct {
  char *command;
  char **args;
  int arg_count;
} Input;

enum {
  FAILURE = -1,
  SUCCESS = 0,
  EXIT = 1,
  RUN = 2,
};

uint64_t get_function_address(GlobalContext *, char *);

void set_breakpoint(uint64_t, GlobalContext *);
void debug_breakpoint(int pid);
void get_breakpoint_context(GlobalContext *);
int handle_input(GlobalContext *, Input *);
void destroy_global_context(GlobalContext *context);
int handle_commands(GlobalContext *context, char *input_buffer);

void reset_breakpoint_data(GlobalContext *);

Input *parse_input(const char *input_line);
void free_input(Input *parsed_input);
Input *get_command(char *buffer, size_t buffer_size);

#define INPUT_BUFFER_LEN 256

#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
      perror(call_description);                                                \
      exit(errno);                                                             \
    }                                                                          \
  } while (0)

#define RETURN_FAILED(cond, retval)                                            \
  do {                                                                         \
    if (cond)                                                                  \
      return retval;                                                           \
  } while (0)

#endif
