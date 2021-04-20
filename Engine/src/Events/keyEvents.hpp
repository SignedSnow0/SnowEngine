#pragma once
#include <iostream>

#include "Event.hpp"

namespace SnowEngine
{
    class KeyPressedEvent : public Event<int, int, int> {
    public:
        KeyPressedEvent() : Event(EventType::Key) {}

        void Dispatch(int key, int scancode, int mods) override {
            if (showToConsole)
                std::cout << "[Key pressed event]: Keycode: " << key << std::endl;

            Event::Dispatch(key, scancode, mods);        
        }
    };

    class KeyReleasedEvent : public Event<int, int, int> {
    public:
        KeyReleasedEvent() : Event(EventType::Key) {}

        void Dispatch(int key, int scancode, int mods) override {
            if (showToConsole)
                std::cout << "[Key released event]: Keycode: " << key << std::endl;

            Event::Dispatch(key, scancode, mods);      
        }
    };

    class KeyCharEvent : public Event<unsigned int> {
    public:
        KeyCharEvent() : Event(EventType::Key) {}

        void Dispatch(unsigned int codepoint) override {
            if (showToConsole)
                std::cout << "[Key char event]: Keycode: " << (char)codepoint << std::endl;

            Event::Dispatch(codepoint);      
        }
    };
}