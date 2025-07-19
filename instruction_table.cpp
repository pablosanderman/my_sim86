#include "instruction_table.h"
#include "sim86.h"

instruction_encoding InstructionTable[] = {
    INST(mov, {B(100010), D, W, MOD, REG, RM}),
};

const char *OpcodeTable[] = {
    "mov",
};