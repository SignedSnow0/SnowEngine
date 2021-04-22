#include "imguiLayer.h"

#include <imguiLib/imgui_impl_vulkan.h>
#include <imguiLib/imgui_impl_glfw.h>

#include "application.h"
#include "input/keyboard.h"

namespace SnowEngine {
	ImGuiLayer::ImGuiLayer(Window& window, Device& device, SwapChain& swapchain) : window(window), device(device), swapchain(swapchain) {
		Init();
		window.GetEvents()->KeyPressedEvent += std::bind(&ImGuiLayer::KeyPressed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	}

	ImGuiLayer::~ImGuiLayer() {
		ImGui_ImplVulkan_Shutdown();
	}

	bool ImGuiLayer::KeyPressed(int key, int scancode, int mods)
	{
		if (Keyboard::IsKeyPressed(SNOW_KEY_ESCAPE)) {
			enabled ? enabled = false : enabled = true;
			enabled ? glfwSetInputMode(window.GetGLFW(), GLFW_CURSOR, GLFW_CURSOR_NORMAL) : glfwSetInputMode(window.GetGLFW(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}

		return !enabled;
	}

	void ImGuiLayer::BeginFrame() {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		if (ImGui::Begin("Statistics")) {
			ImGui::Text("Frametime: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
	}

	void ImGuiLayer::EndFrame(VkCommandBuffer commandBuffer) {
		ImGui::Render();
		if (enabled)
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	}

	void ImGuiLayer::Init() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForVulkan(window.GetGLFW(), true);
		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = device.GetInstance();
		initInfo.PhysicalDevice = device.GetPhysicalDevice();
		initInfo.Device = device.GetDevice();
		initInfo.QueueFamily = device.GetQueueFamilies().graphicsFamily.value();
		initInfo.Queue = device.GetGraphicsQueue();
		initInfo.DescriptorPool = device.GetImGuiDescriptorPool();
		initInfo.MinImageCount = swapchain.GetImageCount();
		initInfo.ImageCount = swapchain.GetImageCount();
		initInfo.Allocator = nullptr;
		initInfo.PipelineCache = nullptr;
		ImGui_ImplVulkan_Init(&initInfo, swapchain.GetRenderPass());

		VkCommandBuffer commandBuffer = device.BeginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		device.EndSingleTimeCommands(commandBuffer);
	}
}