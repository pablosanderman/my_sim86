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

// Memory array for 8086 simulation (64KB address space)
u8 Memory[1024 * 64] = {};

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

// Calculate effective address from memory operand
u32 CalculateEffectiveAddress(instruction_operand *operand, u16 *registers)
{
    if (operand->Type != Operand_Memory)
        return 0;

    effective_address_expression *addr = &operand->Address;
    u32 address = addr->Displacement;

    // Add register terms
    for (int i = 0; i < 2; i++)
    {
        effective_address_term *term = &addr->Terms[i];
        if (term->Register.Index != 0)
        {
            u16 regValue = registers[term->Register.Index - 1];
            address += regValue * term->Scale;
        }
    }

    return address;
}

// Read 16-bit word from memory
u16 ReadMemoryWord(u32 address)
{
    if (address >= sizeof(Memory) - 1)
        return 0;
    return (u16)Memory[address] | ((u16)Memory[address + 1] << 8);
}

// Write 16-bit word to memory
void WriteMemoryWord(u32 address, u16 value)
{
    if (address >= sizeof(Memory) - 1)
        return;
    Memory[address] = (u8)(value & 0xFF);
    Memory[address + 1] = (u8)((value >> 8) & 0xFF);
}

// Get operand value (register, immediate, or memory)
u16 GetOperandValue(instruction_operand *operand, u16 *registers)
{
    switch (operand->Type)
    {
    case Operand_Register:
        return registers[operand->Register.Index - 1];

    case Operand_Immediate:
        return operand->Immediate.Value;

    case Operand_Memory:
    {
        u32 address = CalculateEffectiveAddress(operand, registers);
        return ReadMemoryWord(address);
    }

    default:
        return 0;
    }
}

// Set operand value (register or memory)
void SetOperandValue(instruction_operand *operand, u16 *registers, u16 value)
{
    switch (operand->Type)
    {
    case Operand_Register:
        registers[operand->Register.Index - 1] = value;
        break;

    case Operand_Memory:
    {
        u32 address = CalculateEffectiveAddress(operand, registers);
        WriteMemoryWord(address, value);
        break;
    }

    default:
        break;
    }
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
    switch (Decoded->Op)
    {
    case Op_mov:
    {
        u16 sourceValue = GetOperandValue(&Decoded->Operands[1], Array);
        SetOperandValue(&Decoded->Operands[0], Array, sourceValue);
        // mov doesn't affect flags
        break;
    }

    case Op_add:
    {
        u16 destValue = GetOperandValue(&Decoded->Operands[0], Array);
        u16 sourceValue = GetOperandValue(&Decoded->Operands[1], Array);
        u16 result = destValue + sourceValue;
        SetOperandValue(&Decoded->Operands[0], Array, result);
        UpdateFlags(flags, result);
        break;
    }

    case Op_sub:
    {
        u16 destValue = GetOperandValue(&Decoded->Operands[0], Array);
        u16 sourceValue = GetOperandValue(&Decoded->Operands[1], Array);
        u16 result = destValue - sourceValue;
        SetOperandValue(&Decoded->Operands[0], Array, result);
        UpdateFlags(flags, result);
        break;
    }

    case Op_cmp:
    {
        u16 destValue = GetOperandValue(&Decoded->Operands[0], Array);
        u16 sourceValue = GetOperandValue(&Decoded->Operands[1], Array);
        u16 result = destValue - sourceValue;
        // cmp doesn't modify operands, only flags
        UpdateFlags(flags, result);
        break;
    }

    case Op_jne:
    {
        // Jump if not equal (zero flag is false) - same as jnz
        if (!flags->zero)
        {
            // Relative jump - add the displacement to current IP
            Array[8] = Array[8] + Decoded->Operands[0].Immediate.Value;
        }
        break;
    }

    default:
        break;
    }
}
const char *RegisterTable[] = {"ax", "bx", "cx", "dx", "sp", "bp", "si", "di", "ip"};

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

void PrintExecution(u16 *Array, instruction *Decoded, u16 oldValue, u16 oldIP, processor_flags *oldFlags, processor_flags *newFlags)
{
    if (Decoded->Op == Op_cmp)
    {
        printf(" ; ip:0x%x->0x%x", oldIP, Array[8]);
        PrintFlags(oldFlags, newFlags);
        return;
    }

    if (Decoded->Op == Op_jne)
    {
        printf(" ; ip:0x%x->0x%x", oldIP, Array[8]);
        return;
    }

    // For mov, add, sub operations, show what changed
    if (Decoded->Operands[0].Type == Operand_Register)
    {
        u16 RegisterIndex = Decoded->Operands[0].Register.Index - 1;
        u16 NewValue = Array[RegisterIndex]; // Value after execution
        printf(" ; %s:0x%x->0x%x ip:0x%x->0x%x", RegisterTable[RegisterIndex], oldValue, NewValue, oldIP, Array[8]);
    }
    else
    {
        // Memory destination - just show IP change
        printf(" ; ip:0x%x->0x%x", oldIP, Array[8]);
    }

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

    // Always print IP register
    printf("      %s: 0x%04x (%d)\n", RegisterTable[8], Array[8], Array[8]);

    // Print flags if any are set
    if (flags->sign || flags->zero || flags->parity)
    {
        printf("   flags: ");
        if (flags->sign)
            printf("S");
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
    // char FileName[] = "listing_0046_add_sub_cmp";
    // char FileName[] = "listing_0048_ip_register";
    // char FileName[] = "listing_0049_conditional_jumps";
    // char FileName[] = "listing_0053_add_loop_challenge";
    char FileName[] = "listing_0054_draw_rectangle";

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

    u16 Array[9] = {};
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
                u16 oldIP = At; // Save the IP before execution

                // Get old value of destination operand for printing
                if (Decoded.Operands[0].Type == Operand_Register)
                {
                    oldValue = Array[Decoded.Operands[0].Register.Index - 1];
                }
                else if (Decoded.Operands[0].Type == Operand_Memory)
                {
                    oldValue = GetOperandValue(&Decoded.Operands[0], Array);
                }

                u16 expectedNextIP = At + Decoded.Size;
                Array[8] = expectedNextIP;                                           // Update IP to point to next instruction
                Execute8086Instruction(Array, &Decoded, &flags);                     // Execute and update flags (may modify IP for jumps)
                PrintExecution(Array, &Decoded, oldValue, oldIP, &oldFlags, &flags); // Print with old and new values/flags

                // Check if a jump occurred (IP was modified during execution)
                if (Array[8] != expectedNextIP)
                {
                    // Jump occurred, set At to the new IP value
                    At = Array[8];
                }
                else
                {
                    // No jump, advance normally
                    At += Decoded.Size;
                }
            }
            else
            {
                At += Decoded.Size;
            }
            printf("\n");
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