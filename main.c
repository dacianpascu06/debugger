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

  Elf32_Ehdr *elf_hdr = parse_elf_header(file);
  printf("%d\n", elf_hdr->e_ehsize);
}
