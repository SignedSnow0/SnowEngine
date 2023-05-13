#include "DescriptorSet.h"

#include "Graphics/Vulkan/VkDescriptorSet.h"

namespace SnowEngine
{
	std::shared_ptr<DescriptorSet> DescriptorSet::Create(const std::shared_ptr<const Shader>& shader, const u32 setIndex, const u32 frameCount)
	{
		const auto vkShader = std::static_pointer_cast<const VkShader>(shader);
		return std::make_shared<VkDescriptorSet>(vkShader->Layouts().at(setIndex), frameCount);
	}
}
