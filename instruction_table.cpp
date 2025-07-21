#include "instruction_table.h"
#include "sim86.h"

instruction_encoding InstructionTable[] = {
    INST(mov, {B(100010), D, W, MOD, REG, RM}),
    INST(mov, {B(1011), W, REG, DATA}),
    INST(mov, {B(1011), W, REG, DATA, DATA_IF_W}),
};

const char *OpcodeTable[] = {
    "mov",
};