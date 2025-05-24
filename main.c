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

  ElfFile *elf_file = parse_elf_file(file);
  if (elf_file == NULL) {
    printf("FAILED\n");
  }
  for (int i = 0; i < elf_file->sym_num; i++) {
    printf("%s\n", elf_file->symbol_names[i]);
  }

  rc = destroy_elf_file(elf_file);
  fclose(file);
  return rc;
}
