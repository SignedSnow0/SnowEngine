#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>

#include "Core/Events/KeyEvents.hpp"
#include "Core/Events/MouseEvents.hpp"
#include "Core/Events/OtherEvents.hpp"

namespace SnowEngine {

    class Window {
    private:
        struct Events;
    public:
        Window(int width, int height, const char* title);
        ~Window();

        inline GLFWwindow* GetGLFW() { return this->window; }
        //Return true if the window is closing (x button pressed)
        inline bool         ShouldClose() { return glfwWindowShouldClose(window); }
        //
        inline Events*      GetEvents() { return &windowEvents; }
        //Returns the dimension in pixel of the window
        inline VkExtent2D   GetExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }  
        //Returns the window resized flag, if it returns true the flag is resetted
        inline bool         HasResized() { if (resized) { resized = false; return true; } else return false; }
        
        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

    private:
        void SetupCallbacks();

        inline void SetExtent(int width, int height) { this->width = width; this->height = height; }
        
    private:
        GLFWwindow* window;
        int width;
        int height;
        bool resized = false;

        struct Events
        {
            KeyPressedEvent     KeyPressedEvent{};
            KeyReleasedEvent    KeyReleasedEvent{};
            KeyCharEvent        KeyCharEvent{};

            MousePosEvent       MousePosEvent{};
            MouseEnteredEvent   MouseEnterEvent{};
            MousePressedEvent   MousePressEvent{};
            MouseReleasedEvent  MouseReleaseEvent{};
            MouseScrolledEvent  MouseScrolledEvent{};

            FileDroppedEvent    FileDroppedEvent{};
        } windowEvents;
    };
}