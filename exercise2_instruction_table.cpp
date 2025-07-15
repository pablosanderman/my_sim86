#include <cstdint>
#include <iostream>
#include <sys/resource.h>

// This mimics the pattern from sim86 but simplified
// Your task: Complete this CPU instruction decoding system

enum class OpCode : uint8_t
{
    OP_NOP = 0x90, // Real x86 NOP opcode
    OP_MOV = 0xB0, // Real x86 MOV AL, imm8
    OP_ADD = 0x04, // Real x86 ADD AL, imm8
    OP_SUB = 0x2C, // Real x86 SUB AL, imm8
    OP_JMP = 0xEB, // Real x86 JMP short
    OP_COUNT = 5   // Not an actual opcode
};

enum class InstructionType : uint8_t
{
    TYPE_SIMPLE,    // No operands
    TYPE_REGISTER,  // One register operand
    TYPE_IMMEDIATE, // One immediate value
    TYPE_JUMP,      // Jump address
    TYPE_INVALID
};

struct InstructionInfo
{
    const char *mnemonic;
    InstructionType type;
    uint8_t operand_count;
    uint8_t byte_size;
    uint8_t opcode;
};

// TODO: Create a sparse lookup table for opcodes
// Challenge: OpCode values are now truly sparse (0x04, 0x2C, 0x90, 0xB0, 0xEB)!
// You can't use direct array indexing like Exercise 1. How do you handle this?
// Hint: Switch statement, hash table, or linear search?

const InstructionInfo instructions_table[(uint8_t)OpCode::OP_COUNT] = {
    {"NOP", InstructionType::TYPE_SIMPLE, 0, 1, 0x90},
    {"MOV", InstructionType::TYPE_IMMEDIATE, 1, 2, 0xB0},
    {"ADD", InstructionType::TYPE_IMMEDIATE, 1, 2, 0x04},
    {"SUB", InstructionType::TYPE_IMMEDIATE, 1, 2, 0x2C},
    {"JMP", InstructionType::TYPE_JUMP, 1, 2, 0xEB},
};

// TODO: Implement this function to decode an instruction
InstructionInfo decode_instruction(uint8_t opcode)
{
    for (int i = 0; i < (uint8_t)OpCode::OP_COUNT; i++)
    {
        if (instructions_table[i].opcode == opcode)
        {
            return instructions_table[i];
        }
    }
    return {"INVALID", InstructionType::TYPE_INVALID, 0, 0, 0};
}

// TODO: Create a function that processes an instruction
void execute_instruction(uint8_t opcode, uint8_t operand = 0)
{
    InstructionInfo info = decode_instruction(opcode);

    if (info.type == InstructionType::TYPE_INVALID)
    {
        std::cout << "Invalid opcode: 0x" << std::hex << (int)opcode
                  << std::endl;
        return;
    }

    std::cout << "Executing: " << info.mnemonic;
    if (info.operand_count > 0)
    {
        std::cout << " " << (int)operand;
    }
    std::cout << std::endl;
}

int main()
{
    // Test your implementation
    uint8_t test_codes[] = {0x90, 0xB0, 0x04,
                            0x2C, 0xEB, 0xFF}; // Last one is invalid

    for (uint8_t code : test_codes)
    {
        execute_instruction(code, 42);
    }

    return 0;
}