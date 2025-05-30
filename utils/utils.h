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

uint64_t get_function_address(ElfFile *elf_file, uint64_t base_addr,
                              char *name);

void set_breakpoint(uint64_t address, int pid);
void debug_breakpoint(int pid);
void get_breakpoint_context(int pid, siginfo_t *sig_info,
                            struct user_regs_struct *regs);

void reset_breakpoint_data(int pid, struct user_regs_struct *regs);

extern BreakpointQueue queue;

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
