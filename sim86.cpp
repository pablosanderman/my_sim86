#include "sim86.h"
#include "trace.h"
#include <cstdio>
#include <stdio.h>
#include <string.h>

#include "instruction_table.h"

#include "trace.cpp"

// Binary format: printf("byte: " BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(byte));
#define BYTE_TO_BINARY_PATTERN "%d%d%d%d%d%d%d%d\n"
#define BYTE_TO_BINARY(byte)                                                   \
    ((byte) & 0x80 ? 1 : 0), ((byte) & 0x40 ? 1 : 0), ((byte) & 0x20 ? 1 : 0), \
        ((byte) & 0x10 ? 1 : 0), ((byte) & 0x08 ? 1 : 0),                      \
        ((byte) & 0x04 ? 1 : 0), ((byte) & 0x02 ? 1 : 0),                      \
        ((byte) & 0x01 ? 1 : 0),

const char *RegisterTable[]{
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
};

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

instruction TryDecode(instruction_encoding Pattern, u8 *Bytes, u8 *At)
{
    instruction Inst = {};
    bool Valid = false;
    const char *Opcode = 0;
    bool Has[Bits_Count] = {};
    u32 Value[Bits_Count] = {};
    static char DataBuffer[32]; // Buffer for number-to-string conversion

    u8 BitsIndex = 0;

    for (u8 PatternBitsIndex = 0; PatternBitsIndex < 16; PatternBitsIndex++)
    {
        instruction_bits Bits = Pattern.Bits[PatternBitsIndex];
        instruction_bits_usage Usage = Bits.Usage;

        // Opcode
        if (PatternBitsIndex == 0 && Usage == Bits_Literal)
        {
            u8 Byte = Bytes[*At];
            u8 OpcodeMasked = Byte >> (8 - Bits.Count);
            if (OpcodeMasked == Bits.Value)
            {
                Opcode = OpcodeTable[Pattern.Opcode];
                BitsIndex += Bits.Count;
                continue;
            }
            else
            {
                break;
            }
        }

        Has[Usage] = true;
        u8 Byte = Bytes[*At];
        u8 BitPosition = 8 - BitsIndex - Bits.Count;
        Value[Usage] = (Byte >> BitPosition) & ((1 << Bits.Count) - 1);
        BitsIndex += Bits.Count;

        if (BitsIndex == 8)
        {
            *At += 1;
            BitsIndex = 0;
        }
    }
    if (Valid)
    {
        if (Has[Bits_D] && Has[Bits_RM])
        {
            u8 REGIndex =
                Value[Bits_W] == 1 ? Value[Bits_REG] + 8 : Value[Bits_REG];
            u8 RMIndex =
                Value[Bits_W] == 1 ? Value[Bits_RM] + 8 : Value[Bits_RM];
            const char *regs = RegisterTable[REGIndex];
            const char *rms = RegisterTable[RMIndex];
            Inst = {Opcode, rms, regs};
        }
        else
        {
            u8 REGIndex =
                Value[Bits_W] == 1 ? Value[Bits_REG] + 8 : Value[Bits_REG];
            const char *regs = RegisterTable[REGIndex];
            if (Value[Bits_W])
            {
                u16 bigdata = Value[Bits_DATA] | (Value[Bits_DATA_IF_W] << 8);
                printf("%s %s, %d\n", Opcode, regs, bigdata);
            }
            snprintf(DataBuffer, sizeof(DataBuffer), "%d", Value[Bits_DATA]);
            Inst = {Opcode, regs, DataBuffer};
        }
    }

    return Inst;
}

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

    u8 At = 0;
    while (At < BytesCount)
    {
        for (u8 PatternIndex = 0; PatternIndex < 3; PatternIndex++)
        {
            instruction_encoding Pattern = InstructionTable[PatternIndex];

            instruction Inst = TryDecode(Pattern, Bytes, &At);
            printf("%s %s, %s", Inst.Mnemonic, Inst.Op1, Inst.Op2);
        }
    }

    Trace("\n");

    return 0;
}