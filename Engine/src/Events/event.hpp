#pragma once
#include <map>
#include <functional>
#include <stdexcept>

namespace SnowEngine
{
    enum class EventType {
        Key, Mouse, FileDrop, Update
    };

    template<typename ... TArgs>
    class Event {
    public:
        Event(EventType eventType) : type(eventType) {}
        virtual ~Event() = default;

        inline void operator+=(const std::function<bool(TArgs ...)>& func) { Subscribe(func); }
        inline void operator-=(std::function<bool(TArgs ...)> func) { Unsubscribe(func); }
        inline EventType GetType() { return type; }
        inline void SetShowDebugInfo(bool value) { showToConsole = value; }   

        void Subscribe(const std::function<bool(TArgs ...)>& func) {
            subscribers.push_back(func);
        }

        void Unsubscribe(std::function<bool(TArgs ...)> func = nullptr) {
            if (func != nullptr) {
                int i = 0;
                for (auto fn : subscribers) {
                    if (GetAddress(func) == GetAddress(fn)) {
                        subscribers.erase(subscribers.begin() + i);
                        return;
                    }
                    i++;
                }
                throw std::runtime_error("The function to unsubscribe was never subscribed");
            }
            else
                subscribers.erase(subscribers.end());
        }

    protected:
        virtual void Dispatch(TArgs ... args) {
            for (auto event : subscribers)
                if (!event(args ...))
                    return;
        }

    protected:
        bool showToConsole = false;

    private:
        size_t GetAddress(std::function<bool(TArgs ...)> func) {
            typedef void(fnType)(TArgs ...);
            fnType** fnPointer = func.template target<fnType*>();
            return (size_t)*fnPointer;
        }

    private:
        std::vector<std::function<bool(TArgs ...)>> subscribers;
        EventType type;
    };
}