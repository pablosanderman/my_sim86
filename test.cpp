#include <cstdint>
#include <iostream>

#define B(Bits) {Bits_Literal, sizeof(#Bits) - 1, 0, 0b##Bits}

enum instruction_bits_usage : uint8_t
{
    Bits_Literal = 1,
    // ... other values
};

struct instruction_bits
{
    instruction_bits_usage Usage;
    uint8_t BitCount;
    uint8_t Shift;
    uint8_t Value;
};

int main()
{
    // Test the macro
    std::cout << sizeof("100010") - 1 << std::endl; // 6
    // instruction_bits test = B(100010);

    // std::cout << "Usage: " << (int)test.Usage << std::endl;
    // std::cout << "BitCount: " << (int)test.BitCount << std::endl;
    // std::cout << "Shift: " << (int)test.Shift << std::endl;
    // std::cout << "Value: " << (int)test.Value << std::endl;

    return 0;
}