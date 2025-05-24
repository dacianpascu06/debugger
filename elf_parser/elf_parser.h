#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include <elf.h>
#include <errno.h>
#include <stdio.h>

typedef struct {
  Elf64_Ehdr *elf_hdr;
  Elf64_Shdr **elf_sections;
  // this section contains the string table used to find the names of the
  // sections
  Elf64_Shdr *shstrtab_section;
  char **section_names;
  // this is the section that contains the symbols
  Elf64_Shdr *symbol_section;
  Elf64_Shdr *symbol_str_tbl_section;
  int sym_num;
  Elf64_Sym **symbols;
  char **symbol_names;

} ElfFile;

// returns an ElfFile if successful, otherwise NULL
ElfFile *parse_elf_file(FILE *file);

// returns 0 on succes
int parse_elf_header(FILE *file, ElfFile *elf_file);

// returns 0 on succes
int parse_elf_sections(FILE *file, ElfFile *elf_file);

// returns 0 on success
int check_elf_identification(const Elf64_Ehdr *elf_header);

// returns 0 on success
int parse_elf_shstrtab(FILE *file, ElfFile *elf_file);

// returns 0 on success
int parse_elf_section_names(FILE *file, ElfFile *elf_file);

// returns 0 on success
int parse_symbol_names(FILE *file, ElfFile *elf_file);

// returns 0 on success
int parse_null_terminated_strings(char **, char *, int);

// returns 0 on success
int parse_symbols(FILE *file, ElfFile *elf_file);

// returns 0 on success
int destroy_elf_file(ElfFile *elf_file);

#define RETURN_FAILED(cond, retval)                                            \
  do {                                                                         \
    if (cond)                                                                  \
      return retval;                                                           \
  } while (0)

#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
      perror(call_description);                                                \
      exit(errno);                                                             \
    }                                                                          \
  } while (0)

#endif // ELF_PARSER_H
