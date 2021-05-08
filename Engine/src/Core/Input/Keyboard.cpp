#include "Keyboard.h"
#include <stdexcept>
#include <iostream>

namespace SnowEngine {
    static bool keys[512];

    bool Keyboard::IsKeyPressed(int keycode) {
        return keys[keycode];
    }

    void Keyboard::Init() {
        for (bool& key : keys) key = false;
    }

    void Keyboard::SetKey(int keycode, bool value) {
        try {
            keys[keycode] = value;
        }
        catch (const std::out_of_range& error) {
            std::cerr << "Key pressed with unknown code" << std::endl;
        }  
    }
}