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

#define ImpD(Value) {Bits_D, 1, Value}

enum operation_type : u8
{
    Op_none,

    Op_mov,

    Op_count
};

extern const char *OpcodeTable[];

enum instruction_field_bits_usage : u8
{
    Bits_End,

    Bits_Literal,
    Bits_D,
    Bits_W,
    Bits_MOD,
    Bits_REG,
    Bits_RM,
    Bits_DATA,
    Bits_DATA_IF_W,
    Bits_Disp,

    Bits_Count
};

enum instruction_type : u8
{
    Type_None,
    Type_Immediate,
    Type_Register,
    Type_EAC,
    Type_DirectAccess
};

struct instruction_field_bits
{
    instruction_field_bits_usage Usage;
    u8 Count;
    u8 Value;
};

struct instruction_encoding
{
    operation_type Opcode;
    instruction_field_bits FieldBits[16];
};

struct instruction_extract
{
    bool Has[Bits_Count]; // Bool Array to keep track which fields have matched.
    u32 Raw[Bits_Count];  // u32 Array to keep track of each field's raw data.
};

struct instruction_format
{
    const char *Mnemonic;
    const char *Op1;
    const char *Op2;
};

extern instruction_encoding InstructionTable[];