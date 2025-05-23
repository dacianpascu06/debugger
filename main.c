#include "elf_parser/elf_parser.h"
#include "utils/utils.h"
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  DIE(argc < 2, "Argument number error");

  int rc;

  FILE *file = fopen(argv[1], "r");
  DIE(file == NULL, "fopen error");

  Elf64_Ehdr *elf_hdr = parse_elf_header(file);
  Elf64_Shdr *sym_sec_hdr = get_symbol_tbl_section_hdr(file, elf_hdr);

  int nr_entries = sym_sec_hdr->sh_size / sym_sec_hdr->sh_entsize;
  printf("nr entries %d\n", nr_entries);
}
