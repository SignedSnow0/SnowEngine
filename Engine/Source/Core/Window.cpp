#include "Window.h"

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
	}
}
