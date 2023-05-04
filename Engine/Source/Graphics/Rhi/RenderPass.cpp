#include "RenderPass.h"

#include "Graphics/Vulkan/VkRenderPass.h"

namespace SnowEngine
{
	std::shared_ptr<RenderPass> RenderPass::Create(const std::shared_ptr<const Surface>& surface)
	{
		return std::make_shared<VkRenderPass>(std::static_pointer_cast<const VkSurface>(surface));
	}
}
