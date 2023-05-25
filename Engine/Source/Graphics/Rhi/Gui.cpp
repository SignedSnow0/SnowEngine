#include "Gui.h"

#include "Graphics/Vulkan/VkGui.h"

namespace SnowEngine
{
	b8 Gui::sInitialized{ false };

	std::shared_ptr<Gui> Gui::Create(const std::shared_ptr<const Surface>& surface, const std::shared_ptr<RenderPass>& scene)
	{
		auto gui = std::make_shared<VkGui>(std::static_pointer_cast<const VkSurface>(surface), std::static_pointer_cast<VkRenderPass>(scene));

		if(!sInitialized)
		{
			DarkTheme();
			sInitialized = true;
		}

		return gui;
	}

	void Gui::DarkTheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		auto& colors = style.Colors;

		colors[ImGuiCol_WindowBg] = DarkColors::HexToSrgb(DarkColors::Background4);
		colors[ImGuiCol_ChildBg] = DarkColors::HexToSrgb(DarkColors::Background3);
		colors[ImGuiCol_DockingEmptyBg] = DarkColors::HexToSrgb(DarkColors::Background6);

		colors[ImGuiCol_TitleBg] = DarkColors::HexToSrgb(DarkColors::Background3);
		colors[ImGuiCol_TitleBgActive] = DarkColors::HexToSrgb(DarkColors::Background3Selected);
		colors[ImGuiCol_TitleBgCollapsed] = DarkColors::HexToSrgb(DarkColors::Background3);

		colors[ImGuiCol_Header] = DarkColors::HexToSrgb(DarkColors::Background3);
		colors[ImGuiCol_HeaderHovered] = DarkColors::HexToSrgb(DarkColors::Background3Hover);
		colors[ImGuiCol_HeaderActive] = DarkColors::HexToSrgb(DarkColors::Background3Selected);

		colors[ImGuiCol_Tab] = DarkColors::HexToSrgb(DarkColors::Background3);
		colors[ImGuiCol_TabActive] = DarkColors::HexToSrgb(DarkColors::Background3Pressed);
		colors[ImGuiCol_TabHovered] = DarkColors::HexToSrgb(DarkColors::Background3Hover);
		colors[ImGuiCol_TabUnfocused] = DarkColors::HexToSrgb(DarkColors::Background3);
		colors[ImGuiCol_TabUnfocusedActive] = DarkColors::HexToSrgb(DarkColors::Background3Selected);

		colors[ImGuiCol_Button] = DarkColors::HexToSrgb(DarkColors::Background2);
		colors[ImGuiCol_ButtonHovered] = DarkColors::HexToSrgb(DarkColors::Background2Hover);
		colors[ImGuiCol_ButtonActive] = DarkColors::HexToSrgb(DarkColors::Background2Selected);

		colors[ImGuiCol_FrameBg] = DarkColors::HexToSrgb(DarkColors::Background2);
		colors[ImGuiCol_FrameBgHovered] = DarkColors::HexToSrgb(DarkColors::Background2Hover);
		colors[ImGuiCol_FrameBgActive] = DarkColors::HexToSrgb(DarkColors::Background2Selected);

		colors[ImGuiCol_CheckMark] = DarkColors::HexToSrgb(DarkColors::Background1);
		colors[ImGuiCol_SliderGrab] = DarkColors::HexToSrgb(DarkColors::Background1);
		colors[ImGuiCol_SliderGrabActive] = DarkColors::HexToSrgb(DarkColors::Background1Pressed);

		colors[ImGuiCol_Separator] = DarkColors::HexToSrgb(DarkColors::Background5);
		colors[ImGuiCol_SeparatorActive] = DarkColors::HexToSrgb(DarkColors::Background5Pressed);
		colors[ImGuiCol_SeparatorHovered] = DarkColors::HexToSrgb(DarkColors::Background5Hover);

		style.FrameRounding = 4.0f;
		style.WindowRounding = 4.0f;

		ImGuiIO& io{ ImGui::GetIO() };
		//io.FontGlobalScale = 1.5f;
	}
}
