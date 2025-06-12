#include "elf_parser.h"
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ElfFile *parse_elf_file(FILE *file) {
  int rc;
  ElfFile *elf_file = malloc(sizeof(ElfFile));
  RETURN_FAILED(elf_file == NULL, NULL);

  elf_file->elf_hdr = NULL;
  elf_file->elf_sections = NULL;
  elf_file->section_names = NULL;
  elf_file->symbols = NULL;
  elf_file->symbol_names = NULL;

  rc = parse_elf_header(file, elf_file);
  RETURN_FAILED(rc != 0, NULL);
  rc = check_elf_identification(elf_file->elf_hdr);
  RETURN_FAILED(rc != 0, NULL);
  rc = parse_elf_sections(file, elf_file);
  RETURN_FAILED(rc != 0, NULL);
  rc = parse_elf_shstrtab(file, elf_file);
  RETURN_FAILED(rc != 0, NULL);
  rc = parse_elf_section_names(file, elf_file);
  RETURN_FAILED(rc != 0, NULL);
  rc = parse_symbols(file, elf_file);
  RETURN_FAILED(rc != 0, NULL);
  rc = parse_symbol_names(file, elf_file);
  RETURN_FAILED(rc != 0, NULL);

  return elf_file;
}

int parse_elf_header(FILE *file, ElfFile *elf_file) {

  int rc;
  elf_file->elf_hdr = malloc(sizeof(Elf64_Ehdr));

  RETURN_FAILED(elf_file->elf_hdr == NULL, -1);

  // this value will help restore the file position
  long current_pos = ftell(file);

  rc = fread(elf_file->elf_hdr, sizeof(Elf64_Ehdr), 1, file);
  RETURN_FAILED(rc != 1, -1);

  // reset file position
  rc = fseek(file, current_pos, SEEK_SET);
  RETURN_FAILED(rc != 0, -1);

  return 0;
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
int parse_elf_sections(FILE *file, ElfFile *elf_file) {

  int rc;

  elf_file->elf_sections =
      malloc(sizeof(Elf64_Shdr *) * elf_file->elf_hdr->e_shnum);
  RETURN_FAILED(elf_file->elf_sections == NULL, -1);

  for (int i = 0; i < elf_file->elf_hdr->e_shnum; i++) {
    elf_file->elf_sections[i] = malloc(sizeof(Elf64_Shdr));
    RETURN_FAILED(elf_file->elf_sections[i] == NULL, -1);
  }

  // this value will help restore the file position
  long current_pos = ftell(file);

  rc = fseek(file, elf_file->elf_hdr->e_shoff, SEEK_SET);
  RETURN_FAILED(rc != 0, -1);

  for (int i = 0; i < elf_file->elf_hdr->e_shnum; i++) {
    rc = fread(elf_file->elf_sections[i], sizeof(Elf64_Shdr), 1, file);
    RETURN_FAILED(rc != 1, -1);

    if (elf_file->elf_sections[i]->sh_type == SHT_SYMTAB) {
      elf_file->symbol_section = elf_file->elf_sections[i];
    }
  }

  // reset file position
  rc = fseek(file, current_pos, SEEK_SET);
  RETURN_FAILED(rc != 0, -1);

  return 0;
}

int parse_elf_shstrtab(FILE *file, ElfFile *elf_file) {
  Elf64_Half string_table_index = 0;
  if (elf_file->elf_hdr->e_shstrndx == SHN_UNDEF) {
    // The file has no string section, this is possible if the elf file has been
    // stripped
  } else if (elf_file->elf_hdr->e_shstrndx == SHN_XINDEX) {
    // the index of the string table is found in the first section header
    string_table_index = elf_file->elf_sections[0]->sh_link;
    RETURN_FAILED(string_table_index == 0, -1);
  } else {
    string_table_index = elf_file->elf_hdr->e_shstrndx;
  }
  elf_file->shstrtab_section = elf_file->elf_sections[string_table_index];
  RETURN_FAILED(elf_file->shstrtab_section->sh_type != SHT_STRTAB, -1);
  return 0;
}

int parse_elf_section_names(FILE *file, ElfFile *elf_file) {
  int rc;
  // this value will help restore the file position
  long current_pos = ftell(file);

  // we are gonna read the whole section in a buffer
  char *buffer = malloc(elf_file->shstrtab_section->sh_size);
  RETURN_FAILED(buffer == NULL, -1);

  rc = fseek(file, elf_file->shstrtab_section->sh_offset, SEEK_SET);
  RETURN_FAILED(rc != 0, -1);

  rc = fread(buffer, elf_file->shstrtab_section->sh_size, 1, file);
  RETURN_FAILED(rc != 1, -1);

  // the buffer now contains nr_of_sections * null terminated strings
  elf_file->section_names = malloc(elf_file->elf_hdr->e_shnum * sizeof(char *));
  RETURN_FAILED(elf_file->section_names == NULL, -1);

  rc = extract_section_names(elf_file, buffer);
  RETURN_FAILED(rc != 0, -1);

  free(buffer);

  // reset file position
  rc = fseek(file, current_pos, SEEK_SET);
  RETURN_FAILED(rc != 0, -1);

  return 0;
}

int extract_section_names(ElfFile *elf, char *buffer) {
  for (int i = 0; i < elf->elf_hdr->e_shnum; i++) {

    char *name = buffer + elf->elf_sections[i]->sh_name;
    int len = strlen(name);

    elf->section_names[i] = malloc(len + 1);
    RETURN_FAILED(elf->section_names[i] == NULL, -1);

    memcpy(elf->section_names[i], name, len);
  }
  return 0;
}

int parse_symbols(FILE *file, ElfFile *elf_file) {
  int rc;
  // this value will help restore the file position
  long current_pos = ftell(file);

  // number of symbols
  elf_file->sym_num =
      elf_file->symbol_section->sh_size / elf_file->symbol_section->sh_entsize;

  elf_file->symbols = malloc(sizeof(Elf64_Sym *) * elf_file->sym_num);
  RETURN_FAILED(elf_file->symbols == NULL, -1);

  rc = fseek(file, elf_file->symbol_section->sh_offset, SEEK_SET);
  RETURN_FAILED(rc != 0, -1);

  for (int i = 0; i < elf_file->sym_num; i++) {
    elf_file->symbols[i] = malloc(sizeof(Elf64_Sym));
    RETURN_FAILED(elf_file->symbols[i] == NULL, -1);
    rc = fread(elf_file->symbols[i], sizeof(Elf64_Sym), 1, file);
    RETURN_FAILED(rc != 1, -1);
  }

  elf_file->symbol_str_tbl_section =
      elf_file->elf_sections[elf_file->symbol_section->sh_link];
  RETURN_FAILED(elf_file->symbol_str_tbl_section->sh_type != SHT_STRTAB, -1);

  // reset file position
  rc = fseek(file, current_pos, SEEK_SET);
  RETURN_FAILED(rc != 0, -1);
  return 0;
}

int extract_symbol_names(ElfFile *elf_file, char *buffer) {
  for (int i = 0; i < elf_file->sym_num; i++) {

    char *name = buffer + elf_file->symbols[i]->st_name;
    int len = strlen(name);

    elf_file->symbol_names[i] = calloc(1, len + 1);
    RETURN_FAILED(elf_file->symbol_names[i] == NULL, -1);

    memcpy(elf_file->symbol_names[i], name, len);
  }
  return 0;
}

int parse_symbol_names(FILE *file, ElfFile *elf_file) {
  int rc;

  // check if we have symbols
  if (elf_file->sym_num == 0 || elf_file->symbols == NULL) {
    elf_file->symbol_names = NULL;
    return 0;
  }

  // this value will help restore the file position
  long current_pos = ftell(file);

  elf_file->symbol_names = malloc(sizeof(char *) * elf_file->sym_num);
  RETURN_FAILED(elf_file->symbol_names == NULL, -1);

  char *buffer = malloc(elf_file->symbol_str_tbl_section->sh_size);
  RETURN_FAILED(buffer == NULL, -1);

  rc = fseek(file, elf_file->symbol_str_tbl_section->sh_offset, SEEK_SET);
  RETURN_FAILED(rc != 0, -1);

  // read the symbol names from the file
  rc = fread(buffer, elf_file->symbol_str_tbl_section->sh_size, 1, file);
  RETURN_FAILED(rc != 1, -1);

  rc = extract_symbol_names(elf_file, buffer);
  RETURN_FAILED(rc != 0, -1);

  free(buffer);

  rc = fseek(file, current_pos, SEEK_SET);
  RETURN_FAILED(rc != 0, -1);
  return 0;
}

int destroy_elf_file(ElfFile *elf_file) {
  for (int i = 0; i < elf_file->sym_num; i++) {
    free(elf_file->symbol_names[i]);
    free(elf_file->symbols[i]);
  }
  free(elf_file->symbols);
  free(elf_file->symbol_names);
  for (int i = 0; i < elf_file->elf_hdr->e_shnum; i++) {
    free(elf_file->section_names[i]);
    free(elf_file->elf_sections[i]);
  }
  free(elf_file->section_names);
  free(elf_file->elf_sections);
  free(elf_file->elf_hdr);
  free(elf_file);
  return 0;
}
