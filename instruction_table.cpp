#include "instruction_table.h"
#include "sim86.h"

instruction_encoding InstructionTable[] = {
    INST(mov, {B(100010), D, W, MOD, REG, RM}),
    INST(mov, {B(1011), W, REG, DATA, ImpD(1)}),
    INST(mov, {B(1011), W, REG, DATA, DATA_IF_W, ImpD(1)}),
    INST(mov, {B(1100011), W, MOD, B(000), RM, DATA, ImpD(1)}),
    INST(mov, {B(1100011), W, MOD, B(000), RM, DATA, DATA_IF_W, ImpD(1)})
};

const char *OpcodeTable[] = {
    "none",
    "mov",
};