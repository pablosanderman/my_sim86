# Array Enum Tables Learning Guide

## What Are Enum Tables?

Enum tables are a powerful C++ pattern that uses enums as indices into arrays. This creates fast, cache-friendly lookup tables that map enum values to data structures or other enums.

## Key Concepts

### 1. **Basic Enum-to-Array Mapping**
```cpp
enum Color { RED, GREEN, BLUE, COLOR_COUNT };
const char* color_names[COLOR_COUNT] = {"red", "green", "blue"};
```

### 2. **Sparse vs Dense Tables**
- **Dense**: Enum values are contiguous (0, 1, 2, 3...)
- **Sparse**: Enum values have gaps (0x00, 0x01, 0x04, 0x10...)

### 3. **Multi-dimensional Lookups**
Sometimes you need to look up based on multiple criteria. Think about:
- Hash tables
- Switch statements with multiple cases
- Nested arrays

## Exercise Hints

### Exercise 1 Hints
- Use `(int)InputKey::KEY_COUNT` for array size
- Cast enum values to int for array indexing
- Think about bounds checking with invalid enum values
- For string tables, create another array indexed by the enum

### Exercise 2 Hints
- **Sparse Problem**: Your opcodes are now truly sparse (0x04, 0x2C, 0x90, 0xB0, 0xEB)!
- **Why you can't use direct indexing**: You'd need an array of 256 elements with mostly empty slots
- **Solution Options**:
  1. **Switch statement**: Most straightforward for small number of opcodes
  2. **Linear search**: Simple array of structs, search by opcode
  3. **Hash table**: Map opcode to instruction info
  4. **Lookup function**: Custom mapping logic

### Exercise 3 Hints (The Challenge!)
- **Multi-dimensional Problem**: You need to look up by opcode AND operand types
- **Example queries**: 
  - `find_instruction(0x89, OPERAND_REG, OPERAND_REG)` → MOV reg, reg
  - `find_instruction(0x89, OPERAND_REG, OPERAND_MEM)` → MOV reg, mem
  - `find_instruction(0xB8, OPERAND_REG, OPERAND_IMM)` → MOV reg, imm
- **Approach Ideas**:
  1. **Nested Tables**: Array of arrays
  2. **Flattened Index**: Calculate single index from multiple dimensions
  3. **Multiple Tables**: Separate table for each aspect
  4. **Composite Keys**: Combine criteria into single lookup value
  5. **Linear Search**: Array of structs with all three fields

## Real-World Connection: sim86

Look at the sim86 code you saw earlier. Notice how it uses:
- `instruction_bits_usage` enum for different bit field types
- `instruction_bits` struct to describe bit patterns
- `instruction_encoding` struct that combines operation type with bit patterns
- Static arrays of these structures for lookup

The pattern is: **Enum → Index → Struct → Data**

## Common Pitfalls

1. **Forgetting bounds checking** - What happens with invalid enum values?
2. **Not handling sparse enums** - Direct indexing fails with gaps
3. **Cache misses** - Large sparse tables waste memory
4. **Maintenance** - Adding new enum values requires updating tables

## Advanced Techniques

### Compile-time Validation
```cpp
static_assert(sizeof(lookup_table) / sizeof(lookup_table[0]) == ENUM_COUNT);
```

### Constexpr Tables
```cpp
constexpr auto create_lookup_table() {
    // Build table at compile time
}
```

### Template-based Tables
```cpp
template<typename EnumType>
struct EnumTable {
    // Generic enum table implementation
};
```

## Your Mission

Complete the exercises in order. Each one builds on the previous:
1. **Exercise 1**: Master basic enum tables
2. **Exercise 2**: Handle sparse enums
3. **Exercise 3**: Multi-dimensional lookups

Don't peek at solutions until you've struggled with each problem. The struggle is where the learning happens!

## Questions to Ask Yourself

1. How do you handle invalid enum values?
2. What's the time complexity of your lookup?
3. What's the memory usage of your approach?
4. How would you add new enum values?
5. How does this compare to switch statements?

Good luck! Remember: the goal is to struggle, fail, and then understand deeply. 