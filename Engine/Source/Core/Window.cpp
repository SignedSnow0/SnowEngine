#include "Window.h"

#include "Input.h"

namespace SnowEngine
{
	b8 Window::sGLFWInitialized{ false };

	std::shared_ptr<Window> Window::Create(const char* title, const i32 width, const i32 height)
	{
		return Create(title, width, height, true, true, false);
	}

	std::shared_ptr<Window> Window::Create(const char* title, const i32 width, const i32 height, const b8 resizable, const b8 visible, const b8 maximized)
	{
		return std::make_shared<Window>(title, width, height, resizable, visible, maximized);
	}

	Window::~Window()
	{
		glfwDestroyWindow(mWindow);
	}

	b8 Window::Closing() const { return static_cast<u8>(glfwWindowShouldClose(mWindow)); }

	GLFWwindow* Window::Handle() const { return mWindow; }

	u32 Window::Width() const
	{
		i32 width, height;
		glfwGetWindowSize(mWindow, &width, &height);
		return static_cast<u32>(width);
	}

	u32 Window::Height() const
	{
		i32 width, height;
		glfwGetWindowSize(mWindow, &width, &height);
		return static_cast<u32>(height);
	}

	void Window::Update() { glfwPollEvents(); }

	Window::Window(const char* title, const i32 width, const i32 height, const b8 resizable, const b8 visible, const b8 maximized)
	{
		if (!sGLFWInitialized && !glfwInit())
			return;//TODO: error

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);

		mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

		if (maximized)
			glfwMaximizeWindow(mWindow);

		SetCallbacks();
	}

	void Window::SetCallbacks()
	{
		glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_UNKNOWN)
				return;

			Input::SetKey(static_cast<Key>(key), action != GLFW_RELEASE);
		});

		glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xpos, double ypos)
		{
			Input::SetMousePosition(static_cast<f32>(xpos), static_cast<f32>(ypos));
		});

		glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods)
		{
			Input::SetButton(static_cast<Button>(button), action != GLFW_RELEASE);
		});

		glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double xoffset, double yoffset)
		{
			Input::SetMouseScroll(static_cast<f32>(xoffset), static_cast<f32>(yoffset));
		});
	}
}
