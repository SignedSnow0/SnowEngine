#pragma once
#include <iostream>

#include "Event.hpp"

namespace SnowEngine
{
    class FileDroppedEvent : public Event<int, const char**> {
    public:
        FileDroppedEvent() : Event(EventType::FileDrop) {}

        virtual void Dispatch(int count, const char** paths) override {
            if (showToConsole)
                for (size_t i = 0; i < count; i++)
                    std::cout << "[File drop event]: Path: " << paths[i] << std::endl;

            Event::Dispatch(count, paths);        
        }
    };

    class UpdateEvent : public Event<uint32_t, float> {
    public:
        UpdateEvent() : Event(EventType::Update) {}

        virtual void Dispatch(uint32_t frame, float deltaTime) override {
            if (showToConsole)
                std::cout << "[Update event]: Frame: " << frame << " | Delta time: " << deltaTime << std::endl;

            Event::Dispatch(frame, deltaTime);
        }
    };
}