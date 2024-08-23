#include "back_end/back_end.h"

struct codegen_output *output_code;

void put(int code)
{
    uint8_t *slice = (uint8_t *) &code;
    vector_push_back(output_code->text, slice[0]);
    vector_push_back(output_code->text, slice[1]);
    vector_push_back(output_code->text, slice[2]);
    vector_push_back(output_code->text, slice[3]);
}

/* TODO: back_end_native_claim_reg. */

void back_end_init(struct codegen_output *output)
{
    output_code = output;
    back_end_native_mul(5, 5, 6);
    back_end_native_ret();
    back_end_native_lwu(5, 1000, 0);
}