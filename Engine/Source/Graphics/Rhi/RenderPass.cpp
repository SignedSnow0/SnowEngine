#include "RenderPass.h"

#include "Graphics/Vulkan/VkRenderPass.h"

namespace SnowEngine
{
	std::shared_ptr<RenderPass> RenderPass::Create(const std::shared_ptr<const Surface>& surface, b8 depth)
	{
		return std::make_shared<VkRenderPass>(std::static_pointer_cast<const VkSurface>(surface), depth);
	}

	std::shared_ptr<RenderPass> RenderPass::Create(u32 frameCount, u32 width, u32 height, b8 depth)
	{
		return std::make_shared<VkRenderPass>(frameCount, width, height, depth);
	}
}
