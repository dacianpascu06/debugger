#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include <elf.h>
#include <errno.h>
#include <stdio.h>

// returns the file's elf header
Elf32_Ehdr *parse_elf_header(FILE *file);

// returns 0 if the elf file is truly elf format
int check_elf_identification(const Elf32_Ehdr *elf_header);

// returns the string section header
Elf32_Shdr *get_string_tbl_section_hdr(FILE *file, Elf32_Ehdr *elf_hdr);

#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
      perror(call_description);                                                \
      exit(errno);                                                             \
    }                                                                          \
  } while (0)

#endif // ELF_PARSER_H
