#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <imgui.h>

#include "RenderPass.h"
#include "Surface.h"

namespace SnowEngine
{
	struct DarkColors
	{
		inline static glm::ivec4 Black{ 0x00, 0x00, 0x00, 0xFF };
		inline static glm::ivec4 Grey02{ 0x05, 0x05, 0x05, 0xFF };
		inline static glm::ivec4 Grey04{ 0x0A, 0x0A, 0x0A, 0xFF };
		inline static glm::ivec4 Grey06{ 0x0F, 0x0F, 0x0F, 0xFF };
		inline static glm::ivec4 Grey08{ 0x14, 0x14, 0x14, 0xFF };
		inline static glm::ivec4 Grey10{ 0x1A, 0x1A, 0x1A, 0xFF };
		inline static glm::ivec4 Grey12{ 0x1F, 0x1F, 0x1F, 0xFF };
		inline static glm::ivec4 Grey14{ 0x24, 0x24, 0x24, 0xFF };
		inline static glm::ivec4 Grey16{ 0x29, 0x29, 0x29, 0xFF };
		inline static glm::ivec4 Grey18{ 0x2E, 0x2E, 0x2E, 0xFF };
		inline static glm::ivec4 Grey20{ 0x33, 0x33, 0x33, 0xFF };
		inline static glm::ivec4 Grey22{ 0x38, 0x38, 0x38, 0xFF };
		inline static glm::ivec4 Grey24{ 0x3D, 0x3D, 0x3D, 0xFF };

		inline static glm::ivec4 Red{ 0xBF, 0x1A, 0x2A, 0xFF };
		inline static glm::ivec4 RedHovered{ 0xFF, 0x5A, 0x6A, 0xFF };
		inline static glm::ivec4 RedPressed{ 0xDF, 0x3A, 0x4A, 0xFF };

		inline static glm::ivec4 Green{ 0x50, 0xAE, 0x50, 0xFF };
		inline static glm::ivec4 GreenHovered{ 0x90, 0xEE, 0x90, 0xFF };
		inline static glm::ivec4 GreenPressed{ 0x70, 0xCE, 0x70, 0xFF };

		inline static glm::ivec4 Blue{ 0x40, 0x9E, 0xBF, 0xFF };
		inline static glm::ivec4 BlueHovered{ 0x80, 0xDE, 0xFF, 0xFF };
		inline static glm::ivec4 BluePressed{ 0x60, 0xBE, 0xDF, 0xFF };

		inline static glm::ivec4 Background1 = Grey16;
		inline static glm::ivec4 Background1Hover = Grey24;
		inline static glm::ivec4 Background1Pressed = Grey12;
		inline static glm::ivec4 Background1Selected = Grey22;

		inline static glm::ivec4 Background2 = Grey12;
		inline static glm::ivec4 Background2Hover = Grey20;
		inline static glm::ivec4 Background2Pressed = Grey08;
		inline static glm::ivec4 Background2Selected = Grey18;

		inline static glm::ivec4 Background3 = Grey08;
		inline static glm::ivec4 Background3Hover = Grey16;
		inline static glm::ivec4 Background3Pressed = Grey04;
		inline static glm::ivec4 Background3Selected = Grey14;

		inline static glm::ivec4 Background4 = Grey04;
		inline static glm::ivec4 Background4Hover = Grey12;
		inline static glm::ivec4 Background4Pressed = Black;
		inline static glm::ivec4 Background4Selected = Grey10;

		inline static glm::ivec4 Background5 = Black;
		inline static glm::ivec4 Background5Hover = Grey08;
		inline static glm::ivec4 Background5Pressed = Grey02;
		inline static glm::ivec4 Background5Selected = Grey06;

		inline static glm::ivec4 Background6 = Grey20;

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

	class Gui
	{
	public:
		static std::shared_ptr<Gui> Create(const std::shared_ptr<const Surface>& surface, const std::shared_ptr<RenderPass> &scene);
		virtual ~Gui() = default;

		virtual void Begin(const std::shared_ptr<CommandBuffer>& cmd) = 0;
		virtual void End(const std::shared_ptr<CommandBuffer>& cmd) = 0;

	private:
		static void DarkTheme();

		static b8 sInitialized;
	};
}
