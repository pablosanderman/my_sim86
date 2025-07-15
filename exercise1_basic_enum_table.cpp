#include <cassert>
#include <cstdint>
#include <iostream>

// Your task: Complete this enum table system

enum class InputKey : uint8_t
{
    KEY_W = 0,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_SPACE,
    KEY_COUNT // Always keep this last
};

enum class GameAction : uint8_t
{
    ACTION_MOVE_UP = 0,
    ACTION_MOVE_LEFT,
    ACTION_MOVE_DOWN,
    ACTION_MOVE_RIGHT,
    ACTION_JUMP,
    ACTION_NONE
};

const char *action_names_table[(int)GameAction::ACTION_NONE + 1] = {
    "Move Up",    // ACTION_MOVE_UP = 0
    "Move Left",  // ACTION_MOVE_LEFT = 1
    "Move Down",  // ACTION_MOVE_DOWN = 2
    "Move Right", // ACTION_MOVE_RIGHT = 3
    "Jump",       // ACTION_JUMP = 4
    "None"        // ACTION_NONE = 5
};

// TODO: Create an array that maps InputKey to GameAction
// Hint: Use the enum values as indices
GameAction key_to_action_table[(int)InputKey::KEY_COUNT] = {
    // Fill this in!
    GameAction::ACTION_MOVE_UP,   GameAction::ACTION_MOVE_LEFT,
    GameAction::ACTION_MOVE_DOWN, GameAction::ACTION_MOVE_RIGHT,
    GameAction::ACTION_JUMP,
};

// TODO: Implement this function
GameAction get_action_for_key(InputKey key)
{
    // Your implementation here
    // Handle bounds checking!
    uint8_t index = (uint8_t)key;
    assert(index < (uint8_t)InputKey::KEY_COUNT);

    if (index >= (uint8_t)InputKey::KEY_COUNT)
    {
        return GameAction::ACTION_NONE;
    }

    return key_to_action_table[index];
}

// TODO: Implement this function to print the action name
const char *action_to_string(GameAction action)
{
    // Your implementation here
    // Use an enum-indexed array!
    uint8_t index = (uint8_t)action;
    if (index >= sizeof(action_names_table) / sizeof(action_names_table[0]))
    {
        return "Unknown";
    }
    return action_names_table[index];
}

int main()
{
    // Test your implementation

    InputKey test_keys[] = {InputKey::KEY_W, InputKey::KEY_A, InputKey::KEY_S,
                            InputKey::KEY_D, InputKey::KEY_SPACE};

    for (int i = 0; i < 5; i++)
    {
        GameAction action = get_action_for_key(test_keys[i]);
        std::cout << "Key " << (int)test_keys[i] << " -> "
                  << action_to_string(action) << std::endl;
    }

    return 0;
}