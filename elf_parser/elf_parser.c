#include "elf_parser.h"
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

Elf64_Ehdr *parse_elf_header(FILE *file) {

  int rc;
  Elf64_Ehdr *elf_hdr = malloc(sizeof(Elf64_Ehdr));
  DIE(elf_hdr == NULL, "malloc error");

  // this value will help restore the file position
  long current_pos = ftell(file);

  rc = fread(elf_hdr, sizeof(Elf64_Ehdr), 1, file);
  DIE(rc != 1, "fread error");

  // reset file position
  rc = fseek(file, current_pos, SEEK_SET);
  DIE(rc != 0, "fseek error");

  return elf_hdr;
}

int check_elf_identification(const Elf64_Ehdr *elf_header) {
  if (elf_header->e_ident[0] != 0x7f) {
    return -1;
  }
  if (elf_header->e_ident[1] != 'E') {
    return -1;
  }
  if (elf_header->e_ident[2] != 'L') {
    return -1;
  }
  if (elf_header->e_ident[3] != 'F') {
    return -1;
  }

  return 0;
}

Elf64_Shdr *get_symbol_tbl_section_hdr(FILE *file, Elf64_Ehdr *elf_hdr) {
  int rc;
  // this value will help restore the file position
  long current_pos = ftell(file);
  Elf64_Shdr *section_hdr = malloc(sizeof(Elf64_Shdr));
  DIE(section_hdr == NULL, "malloc error");

  // we will iterate through each section
  // start at the first one
  rc = fseek(file, elf_hdr->e_shoff, SEEK_SET);
  DIE(rc != 0, "fseek error");

  for (int i = 0; i < elf_hdr->e_shnum; i++) {
    rc = fread(section_hdr, sizeof(Elf64_Shdr), 1, file);
    DIE(rc != 1, "fread error");

    if (section_hdr->sh_type == SHT_SYMTAB) {
      break;
    }
  }

  // reset file position
  rc = fseek(file, current_pos, SEEK_SET);
  DIE(rc != 0, "fseek error");

  return section_hdr;
}

Elf64_Shdr *get_string_tbl_section_hdr(FILE *file, Elf64_Ehdr *elf_hdr) {
  // We need to retrieve the string table, which is a section
  // elf_header->e_shstrndx is the index in the section "array"

  int rc;
  Elf64_Half string_table_index = 0;
  // this value will help restore the file position
  long current_pos = ftell(file);
  Elf64_Shdr *section_hdr = malloc(sizeof(Elf64_Shdr));
  DIE(section_hdr == NULL, "malloc error");

  if (elf_hdr->e_shstrndx == SHN_UNDEF) {
    // The file has no string section, this is possible if the elf file has been
    // stripped
  } else if (elf_hdr->e_shstrndx == SHN_XINDEX) {
    // the index of the string table is found in the first section header
    // we move the cursor to the first section header

    rc = fseek(file, elf_hdr->e_shoff, SEEK_SET);
    DIE(rc != 0, "fseek error");

    rc = fread(section_hdr, sizeof(Elf64_Shdr), 1, file);
    DIE(rc < 0, "fread error");

    DIE(section_hdr->sh_link == 0,
        "First section header must hold the value to the string table");
    string_table_index = section_hdr->sh_link;
  } else {
    string_table_index = elf_hdr->e_shstrndx;
  }

  // we set the cursor at the section holding the information about the string
  // table
  rc = fseek(file, elf_hdr->e_shoff + (string_table_index * sizeof(Elf64_Shdr)),
             SEEK_SET);
  DIE(rc != 0, "fseek error");

  // read the string table section header
  rc = fread(section_hdr, sizeof(Elf64_Shdr), 1, file);
  DIE(rc != 1, "fread error");

  DIE(section_hdr->sh_type != SHT_STRTAB, "Section must be string table");

  // reset file position
  rc = fseek(file, current_pos, SEEK_SET);
  DIE(rc != 0, "fseek error");

  return section_hdr;
}
