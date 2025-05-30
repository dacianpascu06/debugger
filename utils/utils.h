#ifndef UTILS_H
#define UTILS_H

#include "../elf_parser/elf_parser.h"
#include <errno.h>
#include <stdint.h>

uint64_t get_function_address(ElfFile *elf_file, uint64_t base_addr,
                              char *name);

#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
      perror(call_description);                                                \
      exit(errno);                                                             \
    }                                                                          \
  } while (0)

#define RETURN_FAILED(cond, retval)                                            \
  do {                                                                         \
    if (cond)                                                                  \
      return retval;                                                           \
  } while (0)

#endif
