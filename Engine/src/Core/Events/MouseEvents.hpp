#pragma once
#include <iostream>

#include "Event.hpp"

namespace SnowEngine
{
    class MousePosEvent : public Event<double, double> {
    public:
        MousePosEvent() : Event(EventType::Mouse) {}

        void Dispatch(double xpos, double ypos) override {
            if (showToConsole)
                std::cout << "[Mouse position event]: Position: " << xpos << " | " << ypos << std::endl;

            Event::Dispatch(xpos, ypos);       
        }
    };

    class MouseEnteredEvent : public Event<bool> {
    public:
        MouseEnteredEvent() : Event(EventType::Mouse) {}

        void Dispatch(bool entered) override {
            if (showToConsole)
                std::cout << "[Mouse enter event]: Entered: " << entered << std::endl;

            Event::Dispatch(entered);         
        }
    };

    class MousePressedEvent : public Event<int, int> {
    public:
        MousePressedEvent() : Event(EventType::Mouse) {}

        void Dispatch(int button, int mods) override {
            if (showToConsole)
                std::cout << "[Mouse press event]: Button: " << button << std::endl;

            Event::Dispatch(button, mods);          
        }
    };

    class MouseReleasedEvent : public Event<int, int> {
    public:
        MouseReleasedEvent() : Event(EventType::Mouse) {}

        void Dispatch(int button, int mods) override {
            if (showToConsole)
                std::cout << "[Mouse release event]: Button: " << button << std::endl;

            Event::Dispatch(button, mods);          
        }
    };

    class MouseScrolledEvent : public Event<double, double> {
    public:
        MouseScrolledEvent() : Event(EventType::Mouse) {}

        void Dispatch(double xoffset, double yoffset) override {
            if (showToConsole)
                std::cout << "[Mouse scroll event]: Delta: " << xoffset << " | " << yoffset << std::endl;

            Event::Dispatch(xoffset, yoffset);           
        }
    };
}