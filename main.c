#include "elf_parser/elf_parser.h"
#include "utils/utils.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

uint64_t base_addr;

void setup(pid_t pid) {
  // we need to find the base virtual address for the file that was given by the
  // ASLR + PIE mechanism
  // we will read /proc/pid/maps and get the first entry
  char file_name[64];
  snprintf(file_name, sizeof(file_name), "/proc/%d/maps", pid);

  FILE *proc_map = fopen(file_name, "r");
  DIE(proc_map == NULL, "error opening file, need sudo perms");

  char line[256];
  fgets(line, sizeof(line), proc_map);
  sscanf(line, "%lx-", &base_addr);
}

char *get_input() {
  char *input = calloc(1, 20);
  size_t input_len;
  getline(&input, &input_len, stdin);
  return input;
}

int main(int argc, char **argv) {
  DIE(argc < 2, "Argument number error");

  struct user_regs_struct regs;

  pid_t pid = fork();

  DIE(pid < -1, "ERROR: Fork");

  if (pid == 0) {
    // signal that the child process wants to be traced
    ptrace(PTRACE_TRACEME);
    execv(argv[1], &argv[2]);
    DIE(1, strerror(errno));
  }

  int *status = calloc(1, sizeof(int));

  // This is the first moment the process is stopped by the kernel
  // with the SISTRAP signal when it detects the execv syscall
  waitpid(pid, status, 0);
  DIE(WIFEXITED(*status), "Exited");

  FILE *file = fopen(argv[1], "r");
  DIE(file == NULL, "error opening file");
  ElfFile *elf_file = parse_elf_file(file);

  // this function will help us calculate the base address in case the file is
  // PIE

  if (elf_file->elf_hdr->e_type != ET_EXEC) {
    setup(pid);
  }

  // function name
  char name[10];
  scanf("%s", name);

  printf("base address 0x%lx\n", base_addr);

  uint64_t address = get_function_address(elf_file, base_addr, name);
  DIE(address == 0, " function not found");
  printf("0x%lx\n", address);

  ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);

  while (waitpid(pid, status, 0) && !WIFEXITED(*status)) {
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);

    if (regs.rax == 60) {
      // exit system call
      ptrace(PTRACE_CONT, pid, NULL, NULL);
      continue;
    }

    // fprintf(stderr, "Value in RAX register :  %llu\n", regs.rax);
    ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);
  }

  destroy_elf_file(elf_file);
}
