#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include <elf.h>
#include <errno.h>
#include <stdio.h>

// returns the file's elf header
Elf64_Ehdr *parse_elf_header(FILE *file);

// returns 0 if the elf file is truly elf format
int check_elf_identification(const Elf64_Ehdr *elf_header);

// returns the string section header
Elf64_Shdr *get_string_tbl_section_hdr(FILE *file, Elf64_Ehdr *elf_hdr);

// returns the symbol table section header
Elf64_Shdr *get_symbol_tbl_section_hdr(FILE *file, Elf64_Ehdr *elf_hdr);

#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
      perror(call_description);                                                \
      exit(errno);                                                             \
    }                                                                          \
  } while (0)

#endif // ELF_PARSER_H
