#include "sim86.h"
#include "trace.h"
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

bool TryDecode(instruction_encoding Pattern, u8 *Bytes, u8 *At)
{
    u8 BitsCount = 0;

    const char *Opcode;
    bool d;
    bool w;
    u8 mod;
    u8 reg;
    u8 rm;

    u8 BitsIndex = 0;

    for (u8 PatternBitsIndex = 0; PatternBitsIndex < 6; PatternBitsIndex++)
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
            }
            else
            {
                return false;
            }
        }

        if (Usage == Bits_D)
        {
            u8 Byte = Bytes[*At];
            u8 BitPosition = 8 - BitsIndex - Bits.Count;
            d = (Byte >> BitPosition) & ((1 << Bits.Count) - 1);
            BitsIndex += Bits.Count;
        }

        if (Usage == Bits_W)
        {
            u8 Byte = Bytes[*At];
            u8 BitPosition = 8 - BitsIndex - Bits.Count;
            w = (Byte >> BitPosition) & ((1 << Bits.Count) - 1);
            BitsIndex += Bits.Count;
        }

        if (Usage == Bits_MOD)
        {
            u8 Byte = Bytes[*At];
            u8 BitPosition = 8 - BitsIndex - Bits.Count;
            mod = (Byte >> BitPosition) & ((1 << Bits.Count) - 1);
            BitsIndex += Bits.Count;
        }

        if (Usage == Bits_REG)
        {
            u8 Byte = Bytes[*At];
            u8 BitPosition = 8 - BitsIndex - Bits.Count;
            reg = (Byte >> BitPosition) & ((1 << Bits.Count) - 1);
            BitsIndex += Bits.Count;
        }

        if (Usage == Bits_RM)
        {
            u8 Byte = Bytes[*At];
            u8 BitPosition = 8 - BitsIndex - Bits.Count;
            rm = (Byte >> BitPosition) & ((1 << Bits.Count) - 1);
            BitsIndex += Bits.Count;
        }

        if (BitsIndex == 8)
        {
            *At += 1;
            BitsIndex = 0;
        }

        BitsCount += Bits.Count;
    }
    u8 REGIndex = w == 1 ? reg + 8 : reg;
    u8 RMIndex = w == 1 ? rm + 8 : rm;
    const char *regs = RegisterTable[REGIndex];
    const char *rms = RegisterTable[RMIndex];
    printf("%s %s, %s\n", Opcode, rms, regs);
    Trace("%s %d %d %x %x %x", Opcode, d, w, mod, reg, rm);
    return false;
}

int main(int ArgCount, char **Args)
{
    // Enable debug tracing
    if (ArgCount > 1 && strcmp(Args[1], "-t") == 0)
    {
        TraceEnabled(1);
    }

    // Load bytes from file
    char FileName[] = "listing_0037_single_register_mov";
    // char FileName[] = "listing_0038_many_register_mov";
    u8 Bytes[1024];
    u32 BytesCount = LoadBytesFromFile(FileName, Bytes);
    Trace("BytesCount: %d\n", BytesCount);

    // Loop over each instruction pattern and pattern match the bytes

    u8 At = 0;
    while (At < BytesCount)
    {
        for (u8 PatternIndex = 0; PatternIndex < 1; PatternIndex++)
        {
            instruction_encoding Pattern = InstructionTable[PatternIndex];

            if (TryDecode(Pattern, Bytes, &At))
            {
                printf("x\n");
            }
        }
    }
    // for (u32 i = 0; i < BytesCount; i += 2)
    // {
    //     u8 byte1 = Bytes[i];
    //     u8 byte2 = Bytes[i + 1];
    //     printf("byte: " BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(byte1));
    //     printf("byte: " BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(byte1));
    // }

    Trace("\n");

    return 0;
}