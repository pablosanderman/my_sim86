#include "sim86_shared.h"
#include <cstring>
#include <stdint.h>
#include <stdio.h>

// Keep the file loading function from your original code
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

// Helper function to print operand
void PrintOperand(instruction_operand *Operand)
{
    switch (Operand->Type)
    {
    case Operand_None:
        break;

    case Operand_Register:
    {
        printf("%s", Sim86_RegisterNameFromOperand(&Operand->Register));
        break;
    }

    case Operand_Memory:
    {
        printf("[");

        effective_address_expression *Address = &Operand->Address;
        bool FirstTerm = true;

        // Print base and index registers
        for (int TermIndex = 0; TermIndex < 2; ++TermIndex)
        {
            effective_address_term *Term = &Address->Terms[TermIndex];
            if (Term->Register.Index != 0)
            {
                if (!FirstTerm)
                {
                    printf(" + ");
                }
                FirstTerm = false;

                printf("%s", Sim86_RegisterNameFromOperand(&Term->Register));
                if (Term->Scale != 1)
                {
                    printf("*%d", Term->Scale);
                }
            }
        }

        // Print displacement
        if (Address->Displacement != 0)
        {
            if (!FirstTerm)
            {
                if (Address->Displacement > 0)
                {
                    printf(" + %d", Address->Displacement);
                }
                else
                {
                    printf(" - %d", -Address->Displacement);
                }
            }
            else
            {
                printf("%d", Address->Displacement);
            }
        }

        printf("]");
        break;
    }

    case Operand_Immediate:
    {
        printf("%d", Operand->Immediate.Value);
        break;
    }
    }
}

void Execute8086Instruction(u16 *Array, instruction *Decoded)
{
    u16 DestIndex = Decoded->Operands[0].Register.Index - 1;
    u16 SourceValue;

    if (Decoded->Op == Op_mov)
    {
        if (Decoded->Operands[1].Type == Operand_Immediate)
        {
            SourceValue = Decoded->Operands[1].Immediate.Value;
        }
        else if (Decoded->Operands[1].Type == Operand_Register)
        {
            SourceValue = Array[Decoded->Operands[1].Register.Index - 1];
        }
        Array[DestIndex] = SourceValue;
    }
    if (Decoded->Op == Op_sub)
    {
        if (Decoded->Operands[1].Type == Operand_Immediate)
        {
            SourceValue = Array[DestIndex] - Decoded->Operands[1].Immediate.Value;
        }
        else if (Decoded->Operands[1].Type == Operand_Register)
        {
            SourceValue = Array[DestIndex] - Array[Decoded->Operands[1].Register.Index - 1];
        }
        Array[DestIndex] = SourceValue;
    }
    if (Decoded->Op == Op_add)
    {
        if (Decoded->Operands[1].Type == Operand_Immediate)
        {
            SourceValue = Array[DestIndex] + Decoded->Operands[1].Immediate.Value;
        }
        else if (Decoded->Operands[1].Type == Operand_Register)
        {
            SourceValue = Array[DestIndex] + Array[Decoded->Operands[1].Register.Index - 1];
        }
        Array[DestIndex] = SourceValue;
    }
    if (Decoded->Op == Op_cmp)
    {
        u32 Value = Array[Decoded->Operands[1].Register.Index - 1] - Array[DestIndex];
        // i dont know how cmp works, does it store a flag? or -1, 0, 1? or subtrack?
    }
}
const char *RegisterTable[] = {
    "ax", "bx", "cx", "dx", "sp", "bp", "si", "di",
};

void PrintExecution(u16 *Array, instruction *Decoded)
{
    u16 RegisterIndex = Decoded->Operands[0].Register.Index - 1;
    u16 OldValue = Array[RegisterIndex]; // Current value (before execution)
    u16 NewValue;
    // I dont know how to store the flags

    if (Decoded->Op == Op_mov)
    {
        if (Decoded->Operands[1].Type == Operand_Immediate)
        {
            NewValue = Decoded->Operands[1].Immediate.Value;
        }
        else if (Decoded->Operands[1].Type == Operand_Register)
        {
            NewValue = Array[Decoded->Operands[1].Register.Index - 1];
        }
    }
    else if (Decoded->Op == Op_sub)
    {
        if (Decoded->Operands[1].Type == Operand_Immediate)
        {
            NewValue = Array[RegisterIndex] - Decoded->Operands[1].Immediate.Value;
        }
        else if (Decoded->Operands[1].Type == Operand_Register)
        {
            NewValue = Array[RegisterIndex] - Array[Decoded->Operands[1].Register.Index - 1];
        }
    }
    else if (Decoded->Op == Op_add)
    {
        if (Decoded->Operands[1].Type == Operand_Immediate)
        {
            NewValue = Array[RegisterIndex] + Decoded->Operands[1].Immediate.Value;
        }
        else if (Decoded->Operands[1].Type == Operand_Register)
        {
            NewValue = Array[RegisterIndex] + Array[Decoded->Operands[1].Register.Index - 1];
        }
    }

    if (Decoded->Op != Op_cmp)
    {
        printf(" ; %s:0x%x->0x%x", RegisterTable[RegisterIndex], OldValue, NewValue);
    }
}

void PrintFinalRegisters(u16 *Array)
{
    printf("\nFinal registers:\n");
    for (int RegisterIndex = 0; RegisterIndex < 8; RegisterIndex++)
    {
        printf("      %s: 0x%04x (%d)\n", RegisterTable[RegisterIndex], Array[RegisterIndex], Array[RegisterIndex]);
    }
}

int main(int ArgCount, char **Args)
{
    // Load bytes from file
    // char FileName[] = "listing_0043_immediate_movs";
    // char FileName[] = "listing_0044_register_movs";
    char FileName[] = "listing_0046_add_sub_cmp";

    u8 Bytes[1024];
    u32 BytesCount = LoadBytesFromFile(FileName, Bytes);

    if (BytesCount == 0)
    {
        return 1;
    }

    b32 Execute = false;

    if (ArgCount == 2)
    {
        char *FileName = Args[1];

        if (strcmp(FileName, "-exec") == 0)
        {
            Execute = true;
        }
    }

    printf("bits 16\n\n");

    u16 Array[8] = {};

    // Decode instructions using shared library
    u32 At = 0;
    while (At < BytesCount)
    {
        instruction Decoded;
        Sim86_Decode8086Instruction(BytesCount - At, Bytes + At, &Decoded);
        if (Decoded.Op && Decoded.Size > 0)
        {
            // Print the instruction
            printf("%s ", Sim86_MnemonicFromOperationType(Decoded.Op));

            // Print operands
            if (Decoded.Operands[0].Type != Operand_None)
            {
                PrintOperand(&Decoded.Operands[0]);

                if (Decoded.Operands[1].Type != Operand_None)
                {
                    printf(", ");
                    PrintOperand(&Decoded.Operands[1]);
                }
            }

            if (Execute)
            {
                PrintExecution(Array, &Decoded);         // Print BEFORE execution to capture old value
                Execute8086Instruction(Array, &Decoded); // Then execute
            }
            printf("\n");
            At += Decoded.Size;
        }
        else
        {
            printf("Unknown instruction at byte %d\n", At);
            break;
        }
    }
    PrintFinalRegisters(Array);

    return 0;
}