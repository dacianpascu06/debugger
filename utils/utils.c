#include "utils.h"
#include <elf.h>
#include <stdint.h>
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

void get_breakpoint_context(int pid, siginfo_t *sig_info,
                            struct user_regs_struct *regs) {
  ptrace(PTRACE_GETREGS, pid, NULL, regs);
  ptrace(PTRACE_GETSIGINFO, pid, NULL, sig_info);
}

void reset_breakpoint_data(int pid, struct user_regs_struct *regs) {
  // dequeue saved data
  Breakpoint bp;
  bp_queue_dequeue(&queue, &bp);

  // backtrack instruction pointer
  regs->rip -= 1;

  DIE(regs->rip != bp.addr,
      "instruction pointer differs from breakpoint address");

  ptrace(PTRACE_SETREGS, pid, NULL, regs);
  ptrace(PTRACE_POKEDATA, pid, regs->rip, bp.orig_instr);
}

uint64_t get_function_address(ElfFile *elf_file, uint64_t base_addr,
                              char *name) {

  int found = -1;
  for (int i = 0; i < elf_file->sym_num; i++) {
    if (!strcmp(elf_file->symbol_names[i], name)) {
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

  return s_hdr->st_value + base_addr;
}

void set_breakpoint(uint64_t address, int pid) {
  uint64_t saved_data = ptrace(PTRACE_PEEKTEXT, pid, (void *)address, NULL);
  u_int64_t data_with_int3 = (saved_data & ~0xFF) | 0xCC;
  bp_queue_enqueue(&queue, address, saved_data);
  int rc =
      ptrace(PTRACE_POKETEXT, pid, (void *)address, (void *)data_with_int3);
  DIE(rc == -1, "error in setting breakpoint");
}
