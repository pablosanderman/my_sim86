#include <cstdint>
#include <iostream>

// This is similar to the actual sim86 pattern you saw in the attached file
// Your task: Create a multi-dimensional enum table system

enum class ProcessorFlags : uint8_t
{
    FLAG_ZERO = 0,
    FLAG_SIGN,
    FLAG_CARRY,
    FLAG_OVERFLOW,
    FLAG_COUNT
};

enum class RegisterSize : uint8_t
{
    SIZE_8BIT = 0,
    SIZE_16BIT,
    SIZE_32BIT,
    SIZE_COUNT
};

enum class OperandType : uint8_t
{
    OPERAND_NONE = 0,
    OPERAND_REG,
    OPERAND_IMM,
    OPERAND_MEM,
    OPERAND_COUNT
};

struct InstructionDescriptor
{
    const char *name;
    uint8_t opcode_base;
    OperandType operand1;
    OperandType operand2;
    RegisterSize default_size;
    bool affects_flags[static_cast<int>(ProcessorFlags::FLAG_COUNT)];
};

// TODO: Create a complex lookup system
// Challenge 1: Multiple opcodes can map to the same instruction with different
// operands Challenge 2: You need to handle flag combinations Challenge 3: Some
// instructions have variants based on operand types

// Hint: You might need multiple tables working together!
const InstructionDescriptor instructions_table[] = {
    // MOV variants
    {"MOV reg, reg",
     0x89,
     OperandType::OPERAND_REG,
     OperandType::OPERAND_REG,
     RegisterSize::SIZE_8BIT,
     {}},
    {"MOV reg, mem",
     0x89,
     OperandType::OPERAND_REG,
     OperandType::OPERAND_MEM,
     RegisterSize::SIZE_8BIT,
     {}},
    {"MOV reg, imm",
     0xB8,
     OperandType::OPERAND_REG,
     OperandType::OPERAND_IMM,
     RegisterSize::SIZE_8BIT,
     {}},
    // ADD variants
    {"ADD reg, reg",
     0x01,
     OperandType::OPERAND_REG,
     OperandType::OPERAND_REG,
     RegisterSize::SIZE_8BIT,
     {}},
    {"ADD reg, imm",
     0x83,
     OperandType::OPERAND_REG,
     OperandType::OPERAND_IMM,
     RegisterSize::SIZE_8BIT,
     {}},
};

// TODO: Implement this function
const InstructionDescriptor *
find_instruction(uint8_t opcode, OperandType op1 = OperandType::OPERAND_NONE,
                 OperandType op2 = OperandType::OPERAND_NONE)
{
    // Your implementation here
    // How do you handle the multi-dimensional lookup?
    for (uint8_t i = 0;
         i < sizeof(instructions_table) / sizeof(instructions_table[0]); i++)
    {
        if (instructions_table[i].opcode_base == opcode &&
            instructions_table[i].operand1 == op1 &&
            instructions_table[i].operand2 == op2)
        {
            return &instructions_table[i]; // Address of table entry
        }
    }
    static const InstructionDescriptor invalid = {"INVALID",
                                                  0x00,
                                                  OperandType::OPERAND_NONE,
                                                  OperandType::OPERAND_NONE,
                                                  RegisterSize::SIZE_COUNT,
                                                  {}};
    return &invalid;
}

// TODO: Implement this function
void print_instruction_info(const InstructionDescriptor *desc)
{
    if (!desc)
    {
        std::cout << "Invalid instruction" << std::endl;
        return;
    }

    std::cout << "Instruction: " << desc->name << std::endl;
    std::cout << "Base opcode: 0x" << std::hex << (int)desc->opcode_base
              << std::endl;
    std::cout << "Operand 1: " << (int)desc->operand1 << std::endl;
    std::cout << "Operand 2: " << (int)desc->operand2 << std::endl;
    std::cout << "Default size: " << (int)desc->default_size << std::endl;

    std::cout << "Affects flags: ";
    for (int i = 0; i < static_cast<int>(ProcessorFlags::FLAG_COUNT); i++)
    {
        if (desc->affects_flags[i])
        {
            std::cout << i << " ";
        }
    }
    std::cout << std::endl;
}

// TODO: Advanced challenge - implement instruction variants
// Some instructions like MOV have multiple opcode variants:
// - MOV reg, reg
// - MOV reg, imm
// - MOV reg, mem
// - MOV mem, reg
// How do you handle this efficiently?

int main()
{
    // Test your multi-dimensional implementation
    std::cout << "Testing multi-dimensional instruction lookup:\n" << std::endl;

    // Test different MOV variants
    std::cout << "MOV variants:" << std::endl;
    const InstructionDescriptor *desc1 = find_instruction(
        0x89, OperandType::OPERAND_REG, OperandType::OPERAND_REG);
    print_instruction_info(desc1);
    std::cout << "---" << std::endl;

    const InstructionDescriptor *desc2 = find_instruction(
        0x89, OperandType::OPERAND_REG, OperandType::OPERAND_MEM);
    print_instruction_info(desc2);
    std::cout << "---" << std::endl;

    const InstructionDescriptor *desc3 = find_instruction(
        0xB8, OperandType::OPERAND_REG, OperandType::OPERAND_IMM);
    print_instruction_info(desc3);
    std::cout << "---" << std::endl;

    // Test ADD variants
    std::cout << "\nADD variants:" << std::endl;
    const InstructionDescriptor *desc4 = find_instruction(
        0x01, OperandType::OPERAND_REG, OperandType::OPERAND_REG);
    print_instruction_info(desc4);
    std::cout << "---" << std::endl;

    const InstructionDescriptor *desc5 = find_instruction(
        0x83, OperandType::OPERAND_REG, OperandType::OPERAND_IMM);
    print_instruction_info(desc5);
    std::cout << "---" << std::endl;

    // Test invalid combination
    std::cout << "\nInvalid combination:" << std::endl;
    const InstructionDescriptor *desc6 = find_instruction(
        0xFF, OperandType::OPERAND_REG, OperandType::OPERAND_REG);
    print_instruction_info(desc6);

    return 0;
}