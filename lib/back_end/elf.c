/* elf.c - ELF header generator.
 * Copyright (C) 2024 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "back_end/elf.h"
#include "util/compiler.h"
#include "util/crc32.h"
#include "util/vector.h"
#include "util/unreachable.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define ELF_PHDR_OFF                0x0040
#define ELF_SH_SIZE                 0x0040
#define ELF_SH_OFF                  0x4000
#define ELF_INIT_SIZE               0x8000
#define ELF_ENTRY_ADDR              0x41000
#define ELF_PHDR_SIZE               0x0038
/* How much bytes occupy one symtab entry. */
#define ELF_SYMTAB_ENTSIZE          24

static int   elf_fd;
static char *elf_map;

#define emit(addr, byte_string) \
    { strcpy(&elf_map[addr], byte_string); }

#define emit_bytes(addr, data) \
    { memcpy(&((uint8_t *) elf_map)[(addr)], (data), sizeof (*(data))); }

static char *emit_symbol(char *start, const char *section)
{
    uint64_t len = strlen(section);
    strcpy(start, section);
    start[len] = 0;
    return start + len + /* NULL byte. */1;
}

static void emit_shdr(uint64_t idx, struct elf_shdr *shdr)
{
    uint64_t shdr_off = ELF_SH_OFF + (ELF_SH_SIZE * idx);

    emit_bytes(shdr_off, shdr);
}

static void emit_phdr(uint64_t idx, struct elf_phdr *phdr)
{
    uint64_t off = ELF_PHDR_OFF + (ELF_PHDR_SIZE * idx);

    emit_bytes(off, phdr);
}

static int dispatch_section_type(const char *name)
{
    if (!strcmp(name, ".text"))
        return SHT_PROGBITS;
    if (!strcmp(name, ".ctors"))
        return /* Wrong. */ SHT_PROGBITS;
    if (!strcmp(name, ".dtors"))
        return /* Wrong. */ SHT_PROGBITS;
    if (!strcmp(name, ".data"))
        return /* Wrong. */ SHT_PROGBITS;
    if (!strcmp(name, ".rodata"))
        return /* Wrong. */ SHT_PROGBITS;
    if (!strcmp(name, ".init_array"))
        return SHT_INIT_ARRAY;
    if (!strcmp(name, ".fini_array"))
        return SHT_FINI_ARRAY;
    if (!strcmp(name, ".strtab"))
        return SHT_STRTAB;
    if (!strcmp(name, ".shstrtab"))
        return SHT_STRTAB;
    if (!strcmp(name, ".symtab"))
        return SHT_SYMTAB;

    weak_unreachable("Don't know which section type assign to `%s`", name);
}

static uint16_t emit_phdrs(uint64_t text_off, uint64_t size)
{
    uint16_t phnum = 0;

    /* TODO: Find out what is Section to Segment mapping. Map
             all needed things. */
    struct elf_phdr phdr = {
        .type   = 0x01,
        .flags  = 7,
        .off    = 0,
        .vaddr  = 0,
        .paddr  = 0x12C,
        .memsz  = 0x12C,
        .filesz = 0x12C,
        .align  = 0x1
    };
    emit_phdr(phnum++, &phdr);

    return phnum;
}

void elf_init(struct elf_entry *e)
{
    elf_fd = open(e->filename, O_CREAT | O_TRUNC | O_RDWR, 0666);

    if (ftruncate(elf_fd, ELF_INIT_SIZE) < 0)
        weak_fatal_errno("ftruncate()");

    elf_map = (char *) mmap(NULL, ELF_INIT_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, elf_fd, 0);
    if (elf_map == MAP_FAILED)
        weak_fatal_errno("mmap()");

    /**********************
     * Stage: Emit section headers
     **********************/

    uint64_t idx = 0;
    uint64_t off = 0;
    uint64_t name_off = 0;
    uint64_t strtab_off = 0;
    uint64_t strtab_idx = 0;
    uint64_t symtab_off = 0;
    uint64_t shstrtab_off = 0;
    uint64_t shstrtab_idx = 0;
    uint64_t text_off = 0;
    uint64_t text_size = 0;
    uint64_t shnum = 0;

    struct elf_shdr null_shdr = {
        0
    };
    emit_shdr(shnum++, &null_shdr);

    idx = 1;
    hashmap_foreach(&e->output.sections, crc, __section) {
        (void) crc;
        struct elf_section *section = (struct elf_section *) __section;

        if (!strcmp(section->name, ".strtab"))
            strtab_idx = idx;

        if (!strcmp(section->name, ".shstrtab"))
            shstrtab_idx = idx;

        ++idx;
    }

    printf(".strtab   idx = %ld\n", strtab_idx);
    printf(".shstrtab idx = %ld\n", shstrtab_idx);

    idx = 1;
    hashmap_foreach(&e->output.sections, crc, __section) {
        (void) crc;
        struct elf_section *section = (struct elf_section *) __section;

        printf("Name: %s\n", section->name);

        struct elf_shdr shdr = {
            .name_ptr = 0x01 + name_off,
            .type     = dispatch_section_type(section->name),
            .addr     = off,
            .off      = off,
            .size     = section->size,
        };

        name_off += strlen(section->name) + /* NULL byte */ 1;

        if (!strcmp(section->name, ".strtab"))
            strtab_off = off;

        if (!strcmp(section->name, ".shstrtab"))
            shstrtab_off = off;

        if (!strcmp(section->name, ".text")) {
            text_off  = off;
            text_size = section->size;
        }

        if (!strcmp(section->name, ".symtab")) {
            shdr.link = shstrtab_idx;
            shdr.entsize = ELF_SYMTAB_ENTSIZE;
            symtab_off = idx;
        }

        emit_shdr(shnum++, &shdr);

        off += section->size;
        ++idx;
    }

    printf(".strtab   off = %lx\n", strtab_off);
    printf(".shstrtab off = %lx\n", shstrtab_off);

    /**********************
     * Stage: Strtab contents
     **********************/
    char *shstrtab_it = &elf_map[shstrtab_off];

    /* Placeholder first symbol, which is empty.
       Required by first NULL section

      [Nr] Name              Type             Address           Offset
           Size              EntSize          Flags  Link  Info  Align
      [ 0]                   NULL             0000000000000000  00000000
           0000000000000000  0000000000000000           0     0     0
    */
    shstrtab_it = emit_symbol(shstrtab_it, "");

    hashmap_foreach(&e->output.sections, crc, __section) {
        (void) crc;
        struct elf_section *section = (struct elf_section *) __section;

        printf("Put to .shstrtab `%s`\n", section->name);

        shstrtab_it = emit_symbol(shstrtab_it, section->name);
    }

    /**********************
     * Stage: Emit program header
     **********************/

     struct elf_fhdr fhdr = {
        .ident     = "\x7F\x45\x4C\x46\x02\x01\x01",
        .type      = ET_EXEC,
        .machine   = ELF_TARGET_ARCH,
        .version   = 2,
        .entry     = ELF_ENTRY_ADDR,
        .phoff     = ELF_PHDR_OFF,
        .shoff     = ELF_SH_OFF,
        .flags     = 0x00,
        .ehsize    = 0x40,
        .phentsize = ELF_PHDR_SIZE,
        .phnum     = emit_phdrs(text_off, text_size), /* TODO: Fix. */
        .shentsize = ELF_SH_SIZE,
        .shnum     = e->output.sections.size,
        .shstrndx  = shstrtab_idx
    };

    emit_bytes(0x00, &fhdr);
}

void elf_exit()
{
    if (munmap(elf_map, ELF_INIT_SIZE) < 0)
        weak_fatal_errno("munmap()");

    if (close(elf_fd) < 0)
        weak_fatal_errno("close()");
}

void elf_init_section(
    struct codegen_output *codegen,
    const char            *section,
    uint64_t               size
) {
    struct elf_section *s   = weak_calloc(1, sizeof (struct elf_section));
    uint64_t            crc = (uint64_t) crc32_string(section);

    s->size = size;
    strncpy(s->name, section, sizeof (s->name) - 1);
    hashmap_put(&codegen->sections, crc, (uintptr_t) s);
}

instr_vector_t *elf_lookup_section(
    struct codegen_output *codegen,
    const char            *section
) {
    uint64_t crc  = (uint64_t) crc32_string(section);
    bool     ok   = 0;
    int64_t  addr = hashmap_get(&codegen->sections, crc, &ok);

    struct elf_section *s = (struct elf_section *) addr;

    return &s->instrs;
}