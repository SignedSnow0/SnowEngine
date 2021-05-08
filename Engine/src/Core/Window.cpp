#include "Window.h"
#include <stdexcept>

#include <Core/ImguiLib/imgui.h>

#include "Core/Input/Keyboard.h"
#include "Core/Input/Mouse.h"
#include "Core/Application.h"

namespace SnowEngine {
    
    static bool glfwInitialized = false;
    
    Window::Window(int width, int height, const char* title) : width(width), height(height) {
        if (!glfwInitialized) {
            glfwInit();
            glfwInitialized = true;
        }
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        
        window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        SetupCallbacks();
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    Window::~Window() {
        glfwDestroyWindow(window);
    }

    void Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create window surface");
    }

    void Window::SetupCallbacks() {
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            auto wnd = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

            wnd->resized = true;
            wnd->SetExtent(width, height);
        });

        glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {   
            Application& app = Application::Get();
            if(key != SNOW_KEY_ESCAPE)
                if (app.ImguiEnabled()) return;

            Window* wnd = (Window*)glfwGetWindowUserPointer(window);
            switch (action)
            {
            case GLFW_PRESS:
            {
                Keyboard::SetKey(key, true);
                wnd->GetEvents()->KeyPressedEvent.Dispatch(key, scancode, mods);
                break;
            }
            case GLFW_REPEAT:
            {
                wnd->GetEvents()->KeyPressedEvent.Dispatch(key, scancode, mods);
                break;
            }
            case GLFW_RELEASE:
            {
                Keyboard::SetKey(key, false);
                wnd->GetEvents()->KeyReleasedEvent.Dispatch(key, scancode, mods);
                break;
            }
            }
        });

        glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codepoint)
        {     
            Application& app = Application::Get();
            if (app.ImguiEnabled()) return;
            Window* wnd = (Window*)glfwGetWindowUserPointer(window);
            wnd->GetEvents()->KeyCharEvent.Dispatch(codepoint);
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos)
        {
            Application& app = Application::Get();
            if (app.ImguiEnabled()) return;
            Window* wnd = (Window*)glfwGetWindowUserPointer(window);
            Mouse::SetPosition({ xpos,ypos });
            wnd->GetEvents()->MousePosEvent.Dispatch(xpos, ypos);
        });

        glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered)
        {
            Window* wnd = (Window*)glfwGetWindowUserPointer(window);
            if (!entered) {
                Mouse::SetPosition({ -1,-1 });
            }
            wnd->GetEvents()->MouseEnterEvent.Dispatch(entered);
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
        {
            Application& app = Application::Get();
            if (app.ImguiEnabled()) return;
            Window* wnd = (Window*)glfwGetWindowUserPointer(window);      
            switch (action)
            {
            case GLFW_PRESS:
            {
                Mouse::SetButton(button, true);
                wnd->GetEvents()->MousePressEvent.Dispatch(button, mods);
                break;
            }
            case GLFW_RELEASE:
            {
                Mouse::SetButton(button, false);
                wnd->GetEvents()->MouseReleaseEvent.Dispatch(button, mods);
                break;
            }
            }
        });

        glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
        {
            Application& app = Application::Get();
            if (app.ImguiEnabled()) return;
            Window* wnd = (Window*)glfwGetWindowUserPointer(window);
            Mouse::SetWheelDelta({ xoffset, yoffset });
            wnd->GetEvents()->MouseScrolledEvent.Dispatch(xoffset, yoffset);
        });

        glfwSetDropCallback(window, [](GLFWwindow* window, int count, const char** paths) //elementi trascinati col cursore dentro la finestra
        {
            Window* wnd = (Window*)glfwGetWindowUserPointer(window);       
            wnd->GetEvents()->FileDroppedEvent.Dispatch(count, paths);
        });
    }
}