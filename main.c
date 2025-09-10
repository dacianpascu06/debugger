#include "elf_parser/elf_parser.h"
#include "utils/breakpoint_queue.h"
#include "utils/utils.h"
#include <bits/types/siginfo_t.h>
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

void setup(GlobalContext *context) {
  // we need to find the base virtual address for the file that was given by the
  // ASLR + PIE mechanism
  // we will read /proc/pid/maps and get the first entry
  char file_name[64];
  snprintf(file_name, sizeof(file_name), "/proc/%d/maps", context->target_pid);

  FILE *proc_map = fopen(file_name, "r");
  DIE(proc_map == NULL, "error opening file, need sudo perms");

  char line[256];
  fgets(line, sizeof(line), proc_map);
  sscanf(line, "%lx-", &context->base_addr);
  fclose(proc_map);
}

void exit_routine(GlobalContext *context, int *status, char *input_buffer,
                  FILE *file) {
  destroy_global_context(context);
  free(status);
  free(input_buffer);
  fclose(file);
}

int main(int argc, char **argv) {
  DIE(argc < 2, "Argument number error");

  pid_t pid = fork();

  DIE(pid < -1, "ERROR: Fork");

  if (pid == 0) {
    // signal that the child process wants to be traced
    ptrace(PTRACE_TRACEME);
    execv(argv[1], &argv[2]);
    DIE(1, strerror(errno));
  }

  // init variables
  int *status = calloc(1, sizeof(int));
  int rc = 0;
  GlobalContext *global_context = calloc(1, (sizeof(GlobalContext)));
  DIE(global_context == NULL, "malloc error");
  global_context->target_pid = pid;
  bp_queue_init(&global_context->bp_queue);
  char *input_buffer = malloc(INPUT_BUFFER_LEN);
  DIE(input_buffer == NULL, "malloc error");

  // This is the first moment the process is stopped by the kernel
  // with the SISTRAP signal when it detects the execv syscall
  waitpid(pid, status, 0);
  DIE(WIFEXITED(*status), "Exited");

  FILE *file = fopen(argv[1], "r");
  DIE(file == NULL, "error opening file");
  global_context->elf_file = parse_elf_file(file);

  // this function will help us calculate the base address in case the file is
  // PIE
  if (global_context->elf_file->elf_hdr->e_type != ET_EXEC) {
    setup(global_context);
  }

  rc = handle_commands(global_context, input_buffer);

  if (rc == EXIT) {
    exit_routine(global_context, status, input_buffer, file);
    return 0;
  }

  ptrace(PTRACE_CONT, pid, NULL, NULL);

  while (waitpid(pid, status, 0) && !WIFEXITED(*status)) {
    debug_breakpoint(global_context->target_pid);
    get_breakpoint_context(global_context);

    DIE(global_context->sig_info.si_signo != SIGTRAP,
        "Stop wasn't triggered by sigtrap");
    reset_breakpoint_data(global_context);

    rc = handle_commands(global_context, input_buffer);

    if (rc == EXIT) {
      exit_routine(global_context, status, input_buffer, file);
      return 0;
    }
    ptrace(PTRACE_CONT, pid, NULL, NULL);
  }

  exit_routine(global_context, status, input_buffer, file);
  return 0;
}
