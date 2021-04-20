#pragma once
#include <imguiLib/imgui.h>

#include "window.h"
#include "device.h"
#include "swapChain.h"

namespace SnowEngine {
	class ImGuiLayer {
	public:
		ImGuiLayer(Window& window, Device& device, SwapChain& swapchain);
		~ImGuiLayer();

		bool KeyPressed(int key, int scancode, int mods);
		void BeginFrame();
		void EndFrame(VkCommandBuffer commandBuffer);
		
		inline void SetEnable(bool value) { enabled = value; }
		inline bool IsEnabled() { return enabled; }

	private:
		void Init();

	private:
		Window& window;
		Device& device;
		SwapChain& swapchain;

		bool enabled = false;
	};
}