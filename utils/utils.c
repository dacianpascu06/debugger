#include "utils.h"
#include <elf.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/user.h>

void debug_breakpoint(int pid) {
  struct user_regs_struct regs;
  siginfo_t sig_info;
  ptrace(PTRACE_GETREGS, pid, NULL, &regs);
  ptrace(PTRACE_GETSIGINFO, pid, NULL, &sig_info);
  printf("stopped at 0x%llx\n", regs.rip);
  printf("code %d\n", sig_info.si_code);
}

void get_breakpoint_context(GlobalContext *context) {
  ptrace(PTRACE_GETREGS, context->target_pid, NULL, &context->regs);
  ptrace(PTRACE_GETSIGINFO, context->target_pid, NULL, &context->sig_info);
}

void reset_breakpoint_data(GlobalContext *context) {
  // dequeue saved data
  Breakpoint bp;
  bp_queue_dequeue(&context->bp_queue, &bp);

  // backtrack instruction pointer
  context->regs.rip -= 1;

  DIE(context->regs.rip != bp.addr,
      "instruction pointer differs from breakpoint address");

  ptrace(PTRACE_SETREGS, context->target_pid, NULL, &context->regs);
  ptrace(PTRACE_POKEDATA, context->target_pid, (void *)context->regs.rip,
         (void *)bp.orig_instr);
}

uint64_t get_function_address(GlobalContext *context, char *name) {

  int found = -1;
  ElfFile *elf_file = context->elf_file;
  for (int i = 0; i < elf_file->sym_num; i++) {
    if (elf_file->symbol_names[i] != NULL &&
        !strcmp(elf_file->symbol_names[i], name)) {
      found = i;
      break;
    }
  }
  RETURN_FAILED(found == -1, 0);
  Elf64_Sym *s_hdr = elf_file->symbols[found];

  // asm doesn't use formal types
  if (ELF64_ST_TYPE(s_hdr->st_info) != STT_NOTYPE) {
    RETURN_FAILED(ELF64_ST_TYPE(s_hdr->st_info) != STT_FUNC, 0);
  }

  return s_hdr->st_value + context->base_addr;
}

void set_breakpoint(uint64_t address, GlobalContext *context) {
  uint64_t saved_data =
      ptrace(PTRACE_PEEKTEXT, context->target_pid, (void *)address, NULL);
  u_int64_t data_with_int3 = (saved_data & ~0xFF) | 0xCC;
  bp_queue_enqueue(&context->bp_queue, address, saved_data);
  int rc = ptrace(PTRACE_POKETEXT, context->target_pid, (void *)address,
                  (void *)data_with_int3);
  DIE(rc == -1, "error in setting breakpoint");
}

Input *parse_input(const char *input_line) {
  if (!input_line)
    return NULL;

  Input *parsed_input = malloc(sizeof(Input));
  if (!parsed_input)
    return NULL;

  parsed_input->command = NULL;
  parsed_input->args = NULL;
  parsed_input->arg_count = 0;

  char *input_copy = strdup(input_line);
  if (!input_copy) {
    free(parsed_input);
    return NULL;
  }

  int token_count = 0;
  char *temp_copy = strdup(input_line);
  char *token = strtok(temp_copy, " \t\n");
  while (token) {
    token_count++;
    token = strtok(NULL, " \t\n");
  }
  free(temp_copy);

  if (token_count == 0) {
    free(input_copy);
    return parsed_input;
  }

  if (token_count > 1) {
    parsed_input->args = malloc((token_count - 1) * sizeof(char *));
    if (!parsed_input->args) {
      free(input_copy);
      free(parsed_input);
      return NULL;
    }
  }

  token = strtok(input_copy, " \t\n");
  if (token) {
    parsed_input->command = strdup(token);
    token = strtok(NULL, " \t\n");

    while (token && parsed_input->arg_count < token_count - 1) {
      parsed_input->args[parsed_input->arg_count] = strdup(token);
      parsed_input->arg_count++;
      token = strtok(NULL, " \t\n");
    }
  }

  free(input_copy);
  return parsed_input;
}

void free_input(Input *parsed_input) {
  if (!parsed_input)
    return;

  if (parsed_input->command) {
    free(parsed_input->command);
  }

  if (parsed_input->args) {
    for (int i = 0; i < parsed_input->arg_count; i++) {
      if (parsed_input->args[i]) {
        free(parsed_input->args[i]);
      }
    }
    free(parsed_input->args);
  }

  free(parsed_input);
}

Input *get_command(char *buffer, size_t buffer_size) {
  printf("(debugger) ");
  if (!fgets(buffer, buffer_size, stdin)) {
    return NULL;
  }
  return parse_input(buffer);
}

int handle_input(GlobalContext *context, Input *in) {
  uint64_t address;

  if (!in->command) {
    return -1;
  }

  if (!strcmp(in->command, "run")) {
    return RUN;
  }

  if (!strcmp(in->command, "exit")) {
    return EXIT;
  }

  if (!strcmp(in->command, "first")) {
    printf("first arg is %llu\n", context->regs.rdi);
    return SUCCESS;
  }
  if (!strcmp(in->command, "second")) {
    printf("second arg is %llu\n", context->regs.rsi);
    return SUCCESS;
  }

  if (!strcmp(in->command, "regs")) {
    printf("Registers:\n");
    printf("RAX: 0x%llx\n", context->regs.rax);
    printf("RBX: 0x%llx\n", context->regs.rbx);
    printf("RCX: 0x%llx\n", context->regs.rcx);
    printf("RDX: 0x%llx\n", context->regs.rdx);
    printf("RSI: 0x%llx\n", context->regs.rsi);
    printf("RDI: 0x%llx\n", context->regs.rdi);
    printf("RBP: 0x%llx\n", context->regs.rbp);
    printf("RSP: 0x%llx\n", context->regs.rsp);
    printf("R8 : 0x%llx\n", context->regs.r8);
    printf("R9 : 0x%llx\n", context->regs.r9);
    printf("R10: 0x%llx\n", context->regs.r10);
    printf("R11: 0x%llx\n", context->regs.r11);
    printf("R12: 0x%llx\n", context->regs.r12);
    printf("R13: 0x%llx\n", context->regs.r13);
    printf("R14: 0x%llx\n", context->regs.r14);
    printf("R15: 0x%llx\n", context->regs.r15);
    printf("RIP: 0x%llx\n", context->regs.rip);
    return SUCCESS;
  }

  if (!strcmp(in->command, "break")) {
    if (in->arg_count == 0 || in->args[0] == NULL) {
      printf("(debugger) Invalid function name\n");
      return FAILURE;
    }
    address = get_function_address(context, in->args[0]);

    if (address == 0) {
      printf("(debugger) Invalid function name \n");
      return FAILURE;
    }

    set_breakpoint(address, context);
    printf("(debugger) Breakpoint set at address 0x%lx\n", address);
    return SUCCESS;
  }

  printf("(debugger) Invalid command\n");
  return FAILURE;
}

int handle_commands(GlobalContext *context, char *input_buffer) {
  int rc;
  while (1) {
    Input *input = get_command(input_buffer, INPUT_BUFFER_LEN);
    rc = handle_input(context, input);
    free_input(input);

    if (rc == EXIT || rc == RUN) {
      return rc;
    }
  }
}

void destroy_global_context(GlobalContext *context) {
  bp_queue_free(&context->bp_queue);
  destroy_elf_file(context->elf_file);
  free(context);
}
