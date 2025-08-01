#include "sim86_shared.h"
#include <cstring>
#include <stdint.h>
#include <stdio.h>

// Flags structure
struct processor_flags
{
    bool zero;
    bool sign;
    bool parity;
};

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

// Helper function to calculate parity (even number of 1s = true)
bool CalculateParity(u16 value)
{
    int count = 0;
    for (int i = 0; i < 8; i++)
    { // Only check low 8 bits for parity
        if (value & (1 << i))
            count++;
    }
    return (count % 2) == 0;
}

// Update flags based on result
void UpdateFlags(processor_flags *flags, u16 result)
{
    flags->zero = (result == 0);
    flags->sign = ((s16)result < 0);
    flags->parity = CalculateParity(result);
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

void Execute8086Instruction(u16 *Array, instruction *Decoded, processor_flags *flags)
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
        // mov doesn't affect flags
    }
    else if (Decoded->Op == Op_sub)
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
        UpdateFlags(flags, SourceValue);
    }
    else if (Decoded->Op == Op_add)
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
        UpdateFlags(flags, SourceValue);
    }
    else if (Decoded->Op == Op_cmp)
    {
        u16 result;
        if (Decoded->Operands[1].Type == Operand_Immediate)
        {
            result = Array[DestIndex] - Decoded->Operands[1].Immediate.Value;
        }
        else if (Decoded->Operands[1].Type == Operand_Register)
        {
            result = Array[DestIndex] - Array[Decoded->Operands[1].Register.Index - 1];
        }
        // cmp doesn't modify registers, only flags
        UpdateFlags(flags, result);
    }
}
const char *RegisterTable[] = {
    "ax", "bx", "cx", "dx", "sp", "bp", "si", "di",
};

// Print flags
void PrintFlags(processor_flags *oldFlags, processor_flags *newFlags)
{
    printf(" flags:");

    // Print old flags
    if (oldFlags->sign)
        printf("S");
    if (oldFlags->zero)
        printf("Z");
    if (oldFlags->parity)
        printf("P");

    printf("->");

    // Print new flags
    if (newFlags->sign)
        printf("S");
    if (newFlags->parity)
        printf("P");
    if (newFlags->zero)
        printf("Z");
}

void PrintExecution(u16 *Array, instruction *Decoded, u16 oldValue, processor_flags *oldFlags, processor_flags *newFlags)
{
    if (Decoded->Op == Op_cmp)
    {
        PrintFlags(oldFlags, newFlags);
        return;
    }

    u16 RegisterIndex = Decoded->Operands[0].Register.Index - 1;
    u16 NewValue = Array[RegisterIndex]; // Value after execution

    printf(" ; %s:0x%x->0x%x", RegisterTable[RegisterIndex], oldValue, NewValue);

    // Check if this operation affects flags and if flags changed
    if (Decoded->Op == Op_sub || Decoded->Op == Op_add)
    {
        if (oldFlags->sign != newFlags->sign || oldFlags->zero != newFlags->zero || oldFlags->parity != newFlags->parity)
        {
            PrintFlags(oldFlags, newFlags);
        }
    }
}

void PrintFinalRegisters(u16 *Array, processor_flags *flags)
{
    printf("\nFinal registers:\n");
    for (int RegisterIndex = 0; RegisterIndex < 8; RegisterIndex++)
    {
        if (Array[RegisterIndex] != 0)
        {
            printf("      %s: 0x%04x (%d)\n", RegisterTable[RegisterIndex], Array[RegisterIndex], Array[RegisterIndex]);
        }
    }

    // Print flags if any are set
    if (flags->sign || flags->zero || flags->parity)
    {
        printf("   flags: ");
        if (flags->parity)
            printf("P");
        if (flags->zero)
            printf("Z");
        printf("\n");
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
    processor_flags flags = {};

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
                processor_flags oldFlags = flags; // Save old flags
                u16 oldValue = 0;
                if (Decoded.Operands[0].Type == Operand_Register)
                {
                    oldValue = Array[Decoded.Operands[0].Register.Index - 1];
                }
                Execute8086Instruction(Array, &Decoded, &flags);              // Execute and update flags
                PrintExecution(Array, &Decoded, oldValue, &oldFlags, &flags); // Print with old and new values/flags
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
    PrintFinalRegisters(Array, &flags);

    return 0;
}