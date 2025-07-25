#include "sim86.h"
#include "trace.h"
#include <cstdio>
#include <stdio.h>
#include <string.h>

#include "instruction_table.h"

#include "trace.cpp"

// Binary format: printf("byte: " BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(byte));
#define BYTE_TO_BINARY_PATTERN "%d%d%d%d%d%d%d%d\n"
#define BYTE_TO_BINARY(byte)                                                                                                                         \
    ((byte) & 0x80 ? 1 : 0), ((byte) & 0x40 ? 1 : 0), ((byte) & 0x20 ? 1 : 0), ((byte) & 0x10 ? 1 : 0), ((byte) & 0x08 ? 1 : 0),                     \
        ((byte) & 0x04 ? 1 : 0), ((byte) & 0x02 ? 1 : 0), ((byte) & 0x01 ? 1 : 0),

const char *RegisterTable[]{
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", "ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
};

const char *BaseAddressTable[] = {"bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"};

// Helper function to build effective address strings
const char *BuildEffectiveAddress(u8 mod, u8 rm, short displacement = 0)
{
    static char addressBuffer[64];

    if (mod == 0b11)
    {
        // This shouldn't be called for register mode, but handle it anyway
        return "ERROR";
    }

    if (mod == 0b00 && rm == 0b110)
    {
        // Direct address: [1234]
        snprintf(addressBuffer, sizeof(addressBuffer), "[%d]", displacement);
        return addressBuffer;
    }

    const char *base = BaseAddressTable[rm];

    if (displacement == 0)
    {
        snprintf(addressBuffer, sizeof(addressBuffer), "[%s]", base);
    }
    else if (displacement > 0)
    {
        snprintf(addressBuffer, sizeof(addressBuffer), "[%s + %d]", base, displacement);
    }
    else
    {
        snprintf(addressBuffer, sizeof(addressBuffer), "[%s - %d]", base, -displacement);
    }

    return addressBuffer;
}

u32 LoadBytesFromFile(char *FileName, u8 Bytes[])
{
    u32 BytesCount = 0;

    FILE *File = fopen(FileName, "rb");
    if (File)
    {
        BytesCount = fread(Bytes, 1, 1024, File);
        fclose(File);
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open %s.\n", FileName);
    }
    return BytesCount;
}

instruction_extract MatchPattern(instruction_encoding Pattern, u8 *Bytes, u32 *At)
{
    instruction_extract Extract = {};

    u8 BitsIndex = 0; // Index to keep track which field we are on, by storing the first bit of the field's index.

    u8 MatchedFieldsCount = 0; // To validate if the pattern matched or not at the end.

    for (u8 PatternFieldBitsIndex = 0; PatternFieldBitsIndex < 16; PatternFieldBitsIndex++)
    {
        instruction_field_bits FieldBits = Pattern.FieldBits[PatternFieldBitsIndex]; // The pattern bits that we are matching against.

        if (FieldBits.Usage == Bits_End)
        {
            break;
        }

        u8 CurrentByte = Bytes[*At];                      // The byte we are pattern matching
        u8 BitPosition = 8 - BitsIndex - FieldBits.Count; // The starting position of the field.

        Extract.Has[FieldBits.Usage] = true;                                                        // Store the presence.
        Extract.Raw[FieldBits.Usage] = (CurrentByte >> BitPosition) & ((1 << FieldBits.Count) - 1); // Store the data.

        if (FieldBits.Usage == Bits_Literal)
        {
            if (Extract.Raw[FieldBits.Usage] != FieldBits.Value)
            {
                return {}; // Means that the opcode doesn't match, so its invalid.
            }
        }

        BitsIndex += FieldBits.Count; // Up the index to the next first bit of the next field's index

        if (BitsIndex == 8)
        {
            *At += 1; // Go to next byte
            BitsIndex = 0;
        }

        // Parse displacement immediately after completing ModR/M byte
        if (FieldBits.Usage == Bits_RM && BitsIndex == 0) // Just finished ModR/M byte
        {
            if (Extract.Has[Bits_MOD])
            {
                u8 mod = Extract.Raw[Bits_MOD];
                u8 rm = Extract.Raw[Bits_RM];

                bool needsDisp8 = (mod == 0b01);
                bool needsDisp16 = (mod == 0b10) || (mod == 0b00 && rm == 0b110);

                if (needsDisp8)
                {
                    Extract.Has[Bits_Disp] = true;
                    Extract.Raw[Bits_Disp] = Bytes[*At];
                    *At += 1;
                }
                else if (needsDisp16)
                {
                    Extract.Has[Bits_Disp] = true;
                    Extract.Raw[Bits_Disp] = Bytes[*At] | (Bytes[*At + 1] << 8); // Little endian
                    *At += 2;
                }
            }
        }
    }

    return Extract; // Return the presence and data per field.
}

const instruction_format InterpretExtract(instruction_extract Extract, operation_type Opcode)
{
    instruction_format Format = {};

    // Opcode
    Format.Mnemonic = OpcodeTable[Opcode];

    // Operands (handle direction flag for proper operand order)
    const char *regOperand = nullptr;
    const char *rmOperand = nullptr;

    if (Extract.Has[Bits_REG])
    {
        u8 RegisterIndex = Extract.Raw[Bits_W] == 1 ? Extract.Raw[Bits_REG] + 8 : Extract.Raw[Bits_REG];
        regOperand = RegisterTable[RegisterIndex];
    }

    if (Extract.Has[Bits_RM])
    {
        u8 mod = Extract.Raw[Bits_MOD];
        u8 rm = Extract.Raw[Bits_RM];

        if (mod == 0b11) // Register Mode
        {
            u8 RegisterIndex = Extract.Raw[Bits_W] == 1 ? rm + 8 : rm;
            rmOperand = RegisterTable[RegisterIndex];
        }
        else // Memory Mode
        {
            u16 displacement = 0;
            if (Extract.Has[Bits_Disp])
            {
                displacement = Extract.Raw[Bits_Disp];
            }
            rmOperand = BuildEffectiveAddress(mod, rm, displacement);
        }
    }

    if (Extract.Has[Bits_D] && Extract.Raw[Bits_D] == 1)
    {
        Format.Op1 = regOperand;
        Format.Op2 = rmOperand;
    }
    else
    {
        Format.Op1 = rmOperand;
        Format.Op2 = regOperand;
    }

    if (Extract.Has[Bits_DATA])
    {
    }

    if (Extract.Has[Bits_DATA_IF_W])
    {
    }

    return Format;
}

// const instruction_format InterpretFields(instruction_extract Extract)
// {
//     bool Valid = false;
//     const char *Opcode = 0;
//     static char DataBuffer[32];

//     // Opcode
//     if (PatternBitsIndex == 0 && Usage == Bits_Literal)
//     {
//         u8 Byte = Bytes[*At];
//         u8 OpcodeMasked = Byte >> (8 - Bits.Count);
//         if (OpcodeMasked == Bits.Value)
//         {
//             Opcode = OpcodeTable[Pattern.Opcode];
//             BitsIndex += Bits.Count;
//             continue;
//         }
//         else
//         {
//             break;
//         }
//     }

//     if (Valid)
//     {
//         if (Has[Bits_D] && Has[Bits_RM])
//         {
//             u8 REGIndex = Value[Bits_W] == 1 ? Value[Bits_REG] + 8 : Value[Bits_REG];
//             u8 RMIndex = Value[Bits_W] == 1 ? Value[Bits_RM] + 8 : Value[Bits_RM];
//             const char *regs = RegisterTable[REGIndex];
//             const char *rms = RegisterTable[RMIndex];
//             Inst = {Opcode, rms, regs};
//         }
//         else
//         {
//             u8 REGIndex = Value[Bits_W] == 1 ? Value[Bits_REG] + 8 : Value[Bits_REG];
//             const char *regs = RegisterTable[REGIndex];
//             if (Value[Bits_W])
//             {
//                 u16 bigdata = Value[Bits_DATA] | (Value[Bits_DATA_IF_W] << 8);
//                 printf("%s %s, %d\n", Opcode, regs, bigdata);
//             }
//             snprintf(DataBuffer, sizeof(DataBuffer), "%d", Value[Bits_DATA]);
//             Inst = {Opcode, regs, DataBuffer};
//         }
//     }
// }

int main(int ArgCount, char **Args)
{
    // Enable debug tracing
    if (ArgCount > 1 && strcmp(Args[1], "-t") == 0)
    {
        TraceEnabled(1);
    }

    // Load bytes from file
    // char FileName[] = "listing_0037_single_register_mov";
    // char FileName[] = "listing_0038_many_register_mov";
    char FileName[] = "listing_0039_more_movs";
    u8 Bytes[1024];
    u32 BytesCount = LoadBytesFromFile(FileName, Bytes);
    Trace("BytesCount: %d\n", BytesCount);

    // Loop over each instruction pattern and pattern match the bytes

    u32 At = 0;
    while (At < BytesCount)
    {
        for (u8 PatternIndex = 0; PatternIndex < 3; PatternIndex++)
        {
            instruction_encoding Pattern = InstructionTable[PatternIndex];

            instruction_extract Extract = MatchPattern(Pattern, Bytes, &At);
            if (Extract.Has[Bits_Literal] == false) // Means the opcode didn't match.
            {
                continue;
            }
            instruction_format Format = InterpretExtract(Extract, Pattern.Opcode);
            printf("%s %s, %s\n", Format.Mnemonic, Format.Op1, Format.Op2);
        }
    }

    Trace("\n");

    return 0;
}