#include "sim86.h"

#define INST(Mnemonic, ...) {Op_##Mnemonic, __VA_ARGS__}
#define B(Bits) {Bits_Literal, sizeof(#Bits) - 1, 0b##Bits}
#define D {Bits_D, 1}
#define W {Bits_W, 1}

#define MOD {Bits_MOD, 2}
#define REG {Bits_REG, 3}
#define RM {Bits_RM, 3}

#define DATA {Bits_DATA, 8}
#define DATA_IF_W {Bits_DATA_IF_W, 8}

enum operation_type : u8
{
    Op_none,
    Op_mov,
    Op_count
};

extern const char *OpcodeTable[];

enum instruction_bits_usage : u8
{
    Bits_Literal,
    Bits_D,
    Bits_W,
    Bits_MOD,
    Bits_REG,
    Bits_RM,
    Bits_DATA,
    Bits_DATA_IF_W,

    Bits_Count
};

struct instruction_bits
{
    instruction_bits_usage Usage;
    u8 Count;
    u8 Value;
};

struct instruction_encoding
{
    operation_type Opcode;
    instruction_bits Bits[16];
};

struct instruction
{
    const char *Mnemonic;
    const char *Op1;
    const char *Op2;
};

extern instruction_encoding InstructionTable[];