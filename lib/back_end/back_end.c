#include "back_end/back_end.h"

struct codegen_output *output_code;

void put(uint8_t *code, uint64_t size)
{
    for (uint64_t i = 0; i < size; ++i)
        vector_push_back(output_code->text, code[i]);
}

/* TODO: back_end_native_claim_reg. */

void back_end_init(struct codegen_output *output)
{
    output_code = output;
}