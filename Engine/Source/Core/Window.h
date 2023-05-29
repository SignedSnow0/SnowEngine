#pragma once
#include <memory>

#include "Core/Types.h"
#include <GLFW/glfw3.h>

namespace SnowEngine
{
	class Window
	{
	public:
		static std::shared_ptr<Window> Create(const char* title, i32 width, i32 height);
		static std::shared_ptr<Window> Create(const char* title, i32 width, i32 height, b8 resizable, b8 visible, b8 maximixed);
		~Window();

		b8 Closing() const;
		GLFWwindow* Handle() const;
		u32 Width() const;
		u32 Height() const;

		static void Update();

		Window(const char* title, i32 width, i32 height, b8 resizable, b8 visible, b8 maximized);

	protected:
		GLFWwindow* mWindow{ nullptr };

		static b8 sGLFWInitialized;

	private:
		void SetCallbacks();
	};
}