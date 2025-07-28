#include "sim86_shared.h"
#include <stdint.h>
#include <stdio.h>

// Keep the file loading function from your original code
uint32_t LoadBytesFromFile(char *FileName, uint8_t Bytes[])
{
    uint32_t BytesCount = 0;

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

int main(int ArgCount, char **Args)
{
    // Load bytes from file
    char FileName[] = "listing_0042_completionist_decode";
    uint8_t Bytes[1024];
    uint32_t BytesCount = LoadBytesFromFile(FileName, Bytes);

    if (BytesCount == 0)
    {
        return 1;
    }

    printf("bits 16\n\n");

    // Decode instructions using shared library
    uint32_t At = 0;
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

            printf("\n");
            At += Decoded.Size;
        }
        else
        {
            printf("Unknown instruction at byte %d\n", At);
            break;
        }
    }

    return 0;
}