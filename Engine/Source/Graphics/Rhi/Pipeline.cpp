#include "Pipeline.h"

#include "Graphics/Vulkan/VkPipeline.h"

namespace SnowEngine
{
	std::shared_ptr<Pipeline> Pipeline::Create(const std::shared_ptr<const Shader>& shader, const std::shared_ptr<const RenderPass>& renderPass, const u32 width, const u32 height)
	{
		return std::make_shared<VkPipeline>(std::static_pointer_cast<const VkShader>(shader), std::static_pointer_cast<const VkRenderPass>(renderPass), width, height);
	}
}
