#include "back_end/back_end.h"
#include "util/compiler.h"

static struct codegen_output *output_code;
static instr_vector_t        *text_section;

void put(uint8_t *code, uint64_t size)
{
    for (uint64_t i = 0; i < size; ++i)
        vector_push_back(*text_section, code[i]);
}

void back_end_init(struct codegen_output *output)
{
    output_code = output;

    hashmap_init(&output_code->fn_offsets, 32);

    text_section = elf_lookup_section(output_code, ".text");
}