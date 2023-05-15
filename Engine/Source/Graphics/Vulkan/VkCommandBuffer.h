#pragma once
#include <vulkan/vulkan.hpp>

#include "Core/Types.h"
#include "Graphics/Rhi/CommandBuffer.h"
#include "VkCore.h"
#include "Graphics/Rhi/Surface.h"

namespace SnowEngine
{
	using vkQueueFamily = u32;
	using vkQueue = std::pair<vkQueueFamily, vk::Queue>;

	class VkCommandBuffer : public CommandBuffer
	{
	public:
		VkCommandBuffer(u32 frameCount, CommandBufferUsage usage);

		vk::Semaphore FinishedSemaphore(u32 frameIndex) const;
		vk::CommandBuffer Buffer(u32 frameIndex) const;
		vk::CommandBuffer CurrentBuffer() const;

		void Begin(u32 currentFrame) const override;
		void End(u32 currentFrame) const override;
		void Submit(u32 currentFrame, const std::shared_ptr<const CommandBuffer>& previousCmd) const override;
		void Submit(u32 currentFrame, const std::shared_ptr<const Surface>& surface) const override;

	private:
		void GetQueue();
		void CreatePool();
		void CreateBuffers(u32 frameCount);
		void CreateSyncData(u32 frameCount);

		struct SyncData
		{
			vk::Fence InFlight;
			vk::Semaphore Finished;
		};
		std::vector<SyncData> mFrames;
		vk::CommandPool mPool;
		std::vector<vk::CommandBuffer> mBuffers;
		vkQueue mQueue;
		CommandBufferUsage mUsage;
	};
}
