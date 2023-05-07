#include "Gui.h"

#include "Graphics/Vulkan/VkGui.h"

namespace SnowEngine
{
	std::shared_ptr<Gui> Gui::Create(const std::shared_ptr<const Surface>& surface, const std::shared_ptr<RenderPass>& scene)
	{
		return std::make_shared<VkGui>(std::static_pointer_cast<const VkSurface>(surface), std::static_pointer_cast<VkRenderPass>(scene));
	}
}
