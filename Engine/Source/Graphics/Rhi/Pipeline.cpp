#include "Pipeline.h"

#include "Graphics/Vulkan/VkPipeline.h"

namespace SnowEngine
{
	PipelineSettings::PipelineSettings(const std::shared_ptr<const SnowEngine::Shader>& shader,
		const std::shared_ptr<const SnowEngine::RenderPass>& renderPass, const u32 width, const u32 height)
		: Shader{ shader }, RenderPass{ renderPass }, Width{ width }, Height{ height } { }

	std::shared_ptr<Pipeline> Pipeline::Create(const PipelineSettings& settings)
	{
		return std::make_shared<VkPipeline>(settings);
	}

	std::shared_ptr<ComputePipeline> ComputePipeline::Create(const std::shared_ptr<const Shader>& shader)
	{
		return std::make_shared<VkComputePipeline>(std::static_pointer_cast<const VkShader>(shader));
	}
}
