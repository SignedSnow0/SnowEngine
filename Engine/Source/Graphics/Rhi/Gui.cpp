#include "Gui.h"

#include <glm/glm.hpp>
#include "Graphics/Vulkan/VkGui.h"

namespace SnowEngine
{
	struct DarkColors
	{
		glm::ivec4 Black{ 0x00, 0x00, 0x00, 0xFF };
		glm::ivec4 Grey02{ 0x05, 0x05, 0x05, 0xFF };
		glm::ivec4 Grey04{ 0x0A, 0x0A, 0x0A, 0xFF };
		glm::ivec4 Grey06{ 0x0F, 0x0F, 0x0F, 0xFF };
		glm::ivec4 Grey08{ 0x14, 0x14, 0x14, 0xFF };
		glm::ivec4 Grey10{ 0x1A, 0x1A, 0x1A, 0xFF };
		glm::ivec4 Grey12{ 0x1F, 0x1F, 0x1F, 0xFF };
		glm::ivec4 Grey14{ 0x24, 0x24, 0x24, 0xFF };
		glm::ivec4 Grey16{ 0x29, 0x29, 0x29, 0xFF };
		glm::ivec4 Grey18{ 0x2E, 0x2E, 0x2E, 0xFF };
		glm::ivec4 Grey20{ 0x33, 0x33, 0x33, 0xFF };
		glm::ivec4 Grey22{ 0x38, 0x38, 0x38, 0xFF };
		glm::ivec4 Grey24{ 0x3D, 0x3D, 0x3D, 0xFF };

		glm::ivec4 Background1 = Grey16;
		glm::ivec4 Background1Hover = Grey24;
		glm::ivec4 Background1Pressed = Grey12;
		glm::ivec4 Background1Selected = Grey22;

		glm::ivec4 Background2 = Grey12;
		glm::ivec4 Background2Hover = Grey20;
		glm::ivec4 Background2Pressed = Grey08;
		glm::ivec4 Background2Selected = Grey18;

		glm::ivec4 Background3 = Grey08;
		glm::ivec4 Background3Hover = Grey16;
		glm::ivec4 Background3Pressed = Grey04;
		glm::ivec4 Background3Selected = Grey14;

		glm::ivec4 Background4 = Grey04;
		glm::ivec4 Background4Hover = Grey12;
		glm::ivec4 Background4Pressed = Black;
		glm::ivec4 Background4Selected = Grey10;

		glm::ivec4 Background5 = Black;
		glm::ivec4 Background5Hover = Grey08;
		glm::ivec4 Background5Pressed = Grey02;
		glm::ivec4 Background5Selected = Grey06;

		glm::ivec4 Background6 = Grey20;

		static ImVec4 HexToSrgb(const glm::ivec4& hex)
		{
			return
			{
				hex.r / 255.0f,
				hex.g / 255.0f,
				hex.b / 255.0f,
				hex.a / 255.0f
			};
		}
	};

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
		const DarkColors darkTheme{};

		ImGuiStyle& style = ImGui::GetStyle();
		auto& colors = style.Colors;

		colors[ImGuiCol_WindowBg] = DarkColors::HexToSrgb(darkTheme.Background4);
		colors[ImGuiCol_ChildBg] = DarkColors::HexToSrgb(darkTheme.Background3);
		colors[ImGuiCol_DockingEmptyBg] = DarkColors::HexToSrgb(darkTheme.Background6);

		colors[ImGuiCol_TitleBg] = DarkColors::HexToSrgb(darkTheme.Background3);
		colors[ImGuiCol_TitleBgActive] = DarkColors::HexToSrgb(darkTheme.Background3Selected);
		colors[ImGuiCol_TitleBgCollapsed] = DarkColors::HexToSrgb(darkTheme.Background3);

		colors[ImGuiCol_Header] = DarkColors::HexToSrgb(darkTheme.Background3);
		colors[ImGuiCol_HeaderHovered] = DarkColors::HexToSrgb(darkTheme.Background3Hover);
		colors[ImGuiCol_HeaderActive] = DarkColors::HexToSrgb(darkTheme.Background3Selected);

		colors[ImGuiCol_Tab] = DarkColors::HexToSrgb(darkTheme.Background3);
		colors[ImGuiCol_TabActive] = DarkColors::HexToSrgb(darkTheme.Background3Pressed);
		colors[ImGuiCol_TabHovered] = DarkColors::HexToSrgb(darkTheme.Background3Hover);
		colors[ImGuiCol_TabUnfocused] = DarkColors::HexToSrgb(darkTheme.Background3);
		colors[ImGuiCol_TabUnfocusedActive] = DarkColors::HexToSrgb(darkTheme.Background3Selected);

		colors[ImGuiCol_Button] = DarkColors::HexToSrgb(darkTheme.Background2);
		colors[ImGuiCol_ButtonHovered] = DarkColors::HexToSrgb(darkTheme.Background2Hover);
		colors[ImGuiCol_ButtonActive] = DarkColors::HexToSrgb(darkTheme.Background2Selected);

		colors[ImGuiCol_FrameBg] = DarkColors::HexToSrgb(darkTheme.Background2);
		colors[ImGuiCol_FrameBgHovered] = DarkColors::HexToSrgb(darkTheme.Background2Hover);
		colors[ImGuiCol_FrameBgActive] = DarkColors::HexToSrgb(darkTheme.Background2Selected);

		colors[ImGuiCol_CheckMark] = DarkColors::HexToSrgb(darkTheme.Background1);
		colors[ImGuiCol_SliderGrab] = DarkColors::HexToSrgb(darkTheme.Background1);
		colors[ImGuiCol_SliderGrabActive] = DarkColors::HexToSrgb(darkTheme.Background1Pressed);

		colors[ImGuiCol_Separator] = DarkColors::HexToSrgb(darkTheme.Background5);
		colors[ImGuiCol_SeparatorActive] = DarkColors::HexToSrgb(darkTheme.Background5Pressed);
		colors[ImGuiCol_SeparatorHovered] = DarkColors::HexToSrgb(darkTheme.Background5Hover);

		style.FrameRounding = 4.0f;
		style.WindowRounding = 4.0f;

		ImGuiIO& io{ ImGui::GetIO() };
		io.FontGlobalScale = 1.5f;
	}
}
