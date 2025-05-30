#include "utils.h"
#include <elf.h>
#include <stdint.h>
#include <string.h>

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
