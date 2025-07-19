#include "sim86.h"

#define INST(Mnemonic, ...) {Op_##Mnemonic, __VA_ARGS__}
#define B(Bits) {Bits_Literal, sizeof(#Bits) - 1, 0b##Bits}
#define D {Bits_D, 1}
#define W {Bits_W, 1}

#define MOD {Bits_MOD, 2}
#define REG {Bits_REG, 3}
#define RM {Bits_RM, 3}

enum instruction_opcode : u8
{
    Op_mov,
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
};

struct instruction_bits
{
    instruction_bits_usage Usage;
    u8 Count;
    u8 Value;
};

struct instruction_encoding
{
    instruction_opcode Opcode;
    instruction_bits Bits[16];
};

extern instruction_encoding InstructionTable[];