#include "sim86.h"
#include "trace.h"
#include <stdio.h>
#include <string.h>

#include "trace.c"

// Lookup tables for instruction decoding
const char *OpCodeTable[] = {
    "add",  "add",  "add",  "add",  "add",  "add",  "push", "pop", "or",
    "or",   "or",   "or",   "or",   "or",   "push", "???",  "adc", "adc",
    "adc",  "adc",  "adc",  "adc",  "push", "pop",  "sbb",  "sbb", "sbb",
    "sbb",  "sbb",  "sbb",  "push", "pop",  "and",  "and",  "mov", "and",
    "and",  "and",  "???",  "daa",  "sub",  "sub",  "sub",  "sub", "sub",
    "sub",  "???",  "das",  "xor",  "xor",  "xor",  "xor",  "xor", "xor",
    "???",  "aaa",  "cmp",  "cmp",  "cmp",  "cmp",  "cmp",  "cmp", "???",
    "aas",  "inc",  "inc",  "inc",  "inc",  "inc",  "inc",  "inc", "inc",
    "dec",  "dec",  "dec",  "dec",  "dec",  "dec",  "dec",  "dec", "push",
    "push", "push", "push", "push", "push", "push", "push", "pop", "pop",
    "pop",  "pop",  "pop",  "pop",  "pop",  "pop",  "???",  "???", "???",
    "???",  "???",  "???",  "???",  "???",  "???",  "???",  "???", "???",
    "???",  "???",  "???",  "???",  "???",  "???",  "???",  "???", "???",
    "???",  "???",  "???",  "???",  "???",  "???",  "???",  "???", "???",
    "???",  "???",  "???",  "???",  "???",  "???",  "???",  "???", "???",
    "???",  "???",  "???",  "???",  "???",  "???",  "???",  "???", "???",
    "mov",  "mov",  "mov",  "mov",  "mov",  "mov",  "mov",  "mov"};

// Register names for W=0 (8-bit registers)
const char *Reg8Table[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

// Register names for W=1 (16-bit registers)
const char *Reg16Table[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "-t") == 0)
    {
        TraceEnabled(1);
    }

    char FileName[] = "listing_0037_single_register_mov";
    u32 BytesRead = 0;

    u8 Buffer[1024];

    FILE *File = fopen(FileName, "rb");
    if (File)
    {
        BytesRead = fread(Buffer, 1, 1024, File);
        fclose(File);
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open %s.\n", FileName);
    }

    Trace("BytesRead: %d\n", BytesRead);
    u8 OpCode = 0;
    u8 D = 0;
    u8 W = 0;
    u8 Mod = 0;
    u8 Reg = 0;
    u8 Rm = 0;
    for (u32 i = 0; i < BytesRead; i++)
    {
        Trace("%02X ", Buffer[i]);

        u8 byte = Buffer[i];
        if (i == 0)
        {
            OpCode = byte >> 2;
            D = (byte >> 1) & 0x1; // Extract D bit properly
            W = byte & 0x1;
            Trace("OpCode: %d, D: %d, W: %d\n", OpCode, D, W);
        }
        else if (i == 1)
        {
            Mod = byte >> 6;
            Reg = (byte >> 3) & 0x7;
            Rm = byte & 0x7;
            Trace("Mod: %d, Reg: %d, Rm: %d\n", Mod, Reg, Rm);

            // Decode the instruction
            if (OpCode < sizeof(OpCodeTable) / sizeof(OpCodeTable[0]))
            {
                const char *mnemonic = OpCodeTable[OpCode];
                const char *regName = W ? Reg16Table[Reg] : Reg8Table[Reg];
                const char *rmName = W ? Reg16Table[Rm] : Reg8Table[Rm];

                // For MOV instruction, determine source and destination based
                // on D bit
                if (D == 1)
                {
                    printf("%s %s, %s\n", mnemonic, regName, rmName);
                }
                else
                {
                    printf("%s %s, %s\n", mnemonic, rmName, regName);
                }
            }
        }
    }

    Trace("\n");

    return 0;
}