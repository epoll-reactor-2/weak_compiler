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

#define emit_bytes_cnt(addr, data, size) \
    { memcpy(&((uint8_t *) elf_map)[(addr)], (data), (size)); }

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

static uint16_t emit_phdrs(uint64_t text_off, uint64_t size, uint64_t sections_size)
{
    uint16_t phnum = 0;

    struct elf_phdr phdr = {
        .type   = PT_LOAD,
        .flags  = PF_R | PF_W,
        .off    = 0x40,
        .vaddr  = 0x40,
        .paddr  = 0x40,
        .memsz  = sections_size,
        .filesz = sections_size,
        .align  = 0x16
    };
    emit_phdr(phnum++, &phdr);

    return phnum;
}

static void calculate_strtabs_index(
    section_vector_t *sections,
    uint64_t         *strtab_idx,
    uint64_t         *shstrtab_idx
) {
    uint64_t idx = 1;

    vector_foreach(*sections, i) {
        struct elf_section *section = &vector_at(*sections, i);

        if (!strcmp(section->name, ".strtab"))
            *strtab_idx = idx;

        if (!strcmp(section->name, ".shstrtab"))
            *shstrtab_idx = idx;

        ++idx;
    }
}

static void emit_shdrs(
    struct codegen_output *output,
    section_vector_t      *sections,
    uint64_t               shstrtab_idx,
    uint64_t              *strtab_off,
    uint64_t              *shstrtab_off,
    uint64_t              *symtab_off,
    uint64_t              *text_off,
    uint64_t              *text_size
) {
    uint64_t idx      = 1;
    uint64_t shnum    = 0;
    uint64_t name_off = 0;
    uint64_t off      = 0;

    struct elf_shdr null_shdr = {
        0
    };
    emit_shdr(shnum++, &null_shdr);

    vector_foreach(*sections, i) {
        struct elf_section *section = &vector_at(*sections, i);

        struct elf_shdr shdr = {
            .name_ptr  = 0x01 + name_off,
            .type      = dispatch_section_type(section->name),
            .addr      = off,
            .off       = off,
            .size      = section->size,
            .flags     = PF_W,
            .addralign = 0x4
        };

        name_off += strlen(section->name) + /* NULL byte */ 1;

        if (!strcmp(section->name, ".strtab"))
            *strtab_off = off;

        if (!strcmp(section->name, ".shstrtab"))
            *shstrtab_off = off;

        if (!strcmp(section->name, ".text")) {
            *text_off  = off;
            *text_size = section->size;
        }

        if (!strcmp(section->name, ".symtab")) {
            shdr.link       = shstrtab_idx;
            shdr.info       = output->syms_cnt;
            shdr.entsize    = ELF_SYMTAB_ENTSIZE;
            *symtab_off     = off;
        }

        emit_shdr(shnum++, &shdr);

        off += section->size;
        ++idx;
    }
}

static void emit_shstrtab(
    section_vector_t *sections,
    uint64_t          shstrtab_off
) {
    char *s = &elf_map[shstrtab_off];

    /* Placeholder first symbol, which is empty.
       Required by first NULL section */
    s = emit_symbol(s, "");

    vector_foreach(*sections, i) {
        struct elf_section *section = &vector_at(*sections, i);

        s = emit_symbol(s, section->name);
    }
}

static void emit_text(
    struct codegen_output *output,
    uint64_t               text_off
) {
    instr_vector_t *text_data = elf_lookup_section(output, ".text");

    /* TODO: Pretty large text section smashes ELF by overriding
             some parts. */
    emit_bytes_cnt(text_off, text_data->data, text_data->size);
}

static void emit_fhdr(
    uint64_t text_off,
    uint64_t text_size,
    uint64_t shstrtab_idx,
    uint64_t off,
    uint64_t shnum
) {
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
        .phnum     = emit_phdrs(text_off, text_size, off),
        .shentsize = ELF_SH_SIZE,
        .shnum     = shnum + /* First NULL section */ 1,
        .shstrndx  = shstrtab_idx
    };

    emit_bytes(0x00, &fhdr);
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
    uint64_t idx            = 0;
    uint64_t off            = ELF_PHDR_OFF;
    uint64_t name_off       = 0;
    uint64_t strtab_off     = 0;
    uint64_t strtab_idx     = 0;
    uint64_t symtab_off     = 0;
    uint64_t shstrtab_off   = 0;
    uint64_t shstrtab_idx   = 0;
    uint64_t text_off       = 0;
    uint64_t text_size      = 0;
    uint64_t shnum          = 0;

    calculate_strtabs_index(
        &e->output.sections,
        &strtab_idx,
        &shstrtab_idx
    );

    emit_shdrs(
        &e->output,
        &e->output.sections,
        shstrtab_idx,
        &strtab_off,
        &shstrtab_off,
        &symtab_off,
        &text_off,
        &text_size
    );

    emit_shstrtab(
        &e->output.sections,
        shstrtab_off
    );

    /**********************
     * Stage: Emit symtab entries
     **********************/

    /* TODO: Collect from fn_offsets for example. */
    struct elf_sym sym = {
        .name   = /* Offset in .shstrtab */ 1,
        .size   = 0,
        .value  = 1,
        .info   = 2,
        .shndx  = shstrtab_idx
    };
    emit_bytes(symtab_off + (ELF_SYMTAB_ENTSIZE * 1), &sym);
    emit_bytes(symtab_off + (ELF_SYMTAB_ENTSIZE * 2), &sym);
    emit_bytes(symtab_off + (ELF_SYMTAB_ENTSIZE * 3), &sym);
    emit_bytes(symtab_off + (ELF_SYMTAB_ENTSIZE * 4), &sym);
    emit_bytes(symtab_off + (ELF_SYMTAB_ENTSIZE * 5), &sym);

    emit_fhdr(
        text_off,
        text_size,
        shstrtab_idx,
        off,
        e->output.sections.count
    );

    emit_text(
        &e->output,
        text_off
    );
}

void elf_exit(struct elf_entry *e)
{
    if (munmap(elf_map, ELF_INIT_SIZE) < 0)
        weak_fatal_errno("munmap()");

    if (close(elf_fd) < 0)
        weak_fatal_errno("close()");

    struct codegen_output *o = &e->output;

    hashmap_destroy(&o->fn_offsets);
    vector_foreach(o->sections, i) {
        struct elf_section *s = &vector_at(o->sections, i);
        vector_free(s->instrs);
    }
    vector_free(o->sections);
}

void elf_init_section(
    struct codegen_output *output,
    const char            *section,
    uint64_t               size
) {
    struct elf_section  s   = {0};
    uint64_t            crc = (uint64_t) crc32_string(section);

    s.size = size;
    strncpy(s.name, section, sizeof (s.name) - 1);
    vector_push_back(output->sections, s);
}

void elf_init_symtab(
    struct codegen_output *output,
    uint64_t               syms_cnt
) {
    output->syms_cnt = syms_cnt;
    elf_init_section(output, ".symtab", syms_cnt * ELF_SYMTAB_ENTSIZE);
}

instr_vector_t *elf_lookup_section(
    struct codegen_output *output,
    const char            *section
) {
    section_vector_t *v = &output->sections;

    vector_foreach(*v, i) {
        struct elf_section *s = &vector_at(*v, i);

        if (!strcmp(s->name, section))
            return &s->instrs;
    }

    weak_unreachable("Cannot find `%s` section", section);
}

/*
Огненный гром
Уничтожил твой дом,
Но в руинах партера
На мёрзлой земле
Ты исполнила танец,
Танец на битом стекле.
Танец на битом стекле,
На битом стекле босиком.
*/