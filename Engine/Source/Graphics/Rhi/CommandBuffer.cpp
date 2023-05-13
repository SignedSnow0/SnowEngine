#include "CommandBuffer.h"

#include "Graphics/Vulkan/VkCommandBuffer.h"

namespace SnowEngine
{
	std::shared_ptr<CommandBuffer> CommandBuffer::Create(const u32 frameCount, const CommandBufferUsage usage)
	{
		return std::make_shared<VkCommandBuffer>(frameCount, usage);
	}
}
