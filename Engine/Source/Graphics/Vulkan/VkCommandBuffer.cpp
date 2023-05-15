#include "VkCommandBuffer.h"

#include "VkSurface.h"

namespace SnowEngine
{
	VkCommandBuffer::VkCommandBuffer(const u32 frameCount, const CommandBufferUsage usage)
		: mUsage{ usage }
	{
		GetQueue();
		CreatePool();
		CreateBuffers(frameCount);
		CreateSyncData(frameCount);
	}

	vk::Semaphore VkCommandBuffer::FinishedSemaphore(const u32 frameIndex) const { return mFrames[frameIndex].Finished; }

	vk::CommandBuffer VkCommandBuffer::Buffer(const u32 frameIndex) const { return mBuffers[frameIndex]; }

	vk::CommandBuffer VkCommandBuffer::CurrentBuffer() const { return mBuffers[VkSurface::BoundSurface()->CurrentFrame()]; }

	void VkCommandBuffer::Begin(const u32 currentFrame) const
	{
		vk::Result result = VkCore::Get()->Device().waitForFences(mFrames[currentFrame].InFlight, true, std::numeric_limits<u64>::max());

		//TODO: resource updates here

		VkCore::Get()->Device().resetFences(mFrames[currentFrame].InFlight);

		vk::CommandBufferBeginInfo beginInfo{};

		mBuffers[currentFrame].reset();
		mBuffers[currentFrame].begin(beginInfo);
	}

	void VkCommandBuffer::End(const u32 currentFrame) const
	{
		mBuffers[currentFrame].end();
	}

	void VkCommandBuffer::Submit(const u32 currentFrame, const std::shared_ptr<const CommandBuffer>& previousCmd) const
	{
		std::vector<vk::Semaphore> wait{};
		std::vector<vk::PipelineStageFlags> stages{};

		if (previousCmd != nullptr)
		{
			const auto& vkPreviousCmd = std::static_pointer_cast<const VkCommandBuffer>(previousCmd);
			wait.emplace_back(vkPreviousCmd->mFrames[currentFrame].Finished);

			stages.emplace_back(vk::PipelineStageFlagBits::eVertexInput);
			stages.emplace_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		}

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mBuffers[currentFrame];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &mFrames[currentFrame].Finished;
		submitInfo.waitSemaphoreCount = static_cast<u32>(wait.size());
		submitInfo.pWaitSemaphores = wait.data();
		submitInfo.pWaitDstStageMask = stages.data();

		VkCore::Get()->Queues().Compute.second.submit(submitInfo, mFrames[currentFrame].InFlight);
	}

	void VkCommandBuffer::Submit(u32 currentFrame, const std::shared_ptr<const Surface>& surface) const
	{
		std::vector<vk::Semaphore> wait{};
		std::vector<vk::PipelineStageFlags> stages{};

		if (surface != nullptr)
		{
			const auto& vkSurface = std::static_pointer_cast<const VkSurface>(surface);
			wait.emplace_back(vkSurface->ImageAvailableSemaphore());

			stages.emplace_back(vk::PipelineStageFlagBits::eVertexInput);
			stages.emplace_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		}

		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mBuffers[currentFrame];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &mFrames[currentFrame].Finished;
		submitInfo.waitSemaphoreCount = static_cast<u32>(wait.size());
		submitInfo.pWaitSemaphores = wait.data();
		submitInfo.pWaitDstStageMask = stages.data();

		VkCore::Get()->Queues().Compute.second.submit(submitInfo, mFrames[currentFrame].InFlight);
	}

	void VkCommandBuffer::GetQueue()
	{
		switch (mUsage)
		{
		case CommandBufferUsage::Compute:
			mQueue = VkCore::Get()->Queues().Compute;
			break;
		case CommandBufferUsage::Graphics:
			mQueue = VkCore::Get()->Queues().Graphics;
			break;
		case CommandBufferUsage::Copy:
			mQueue = VkCore::Get()->Queues().Graphics;
			break;
		}
	}

	void VkCommandBuffer::CreatePool()
	{
		vk::CommandPoolCreateInfo createInfo{};
		createInfo.queueFamilyIndex = mQueue.first;
		createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		
		mPool = VkCore::Get()->Device().createCommandPool(createInfo);
	}

	void VkCommandBuffer::CreateBuffers(const u32 frameCount)
	{
		vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.commandPool = mPool;
		commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
		commandBufferAllocateInfo.commandBufferCount = frameCount;

		mBuffers = VkCore::Get()->Device().allocateCommandBuffers(commandBufferAllocateInfo);
	}

	void VkCommandBuffer::CreateSyncData(const u32 frameCount)
	{
		vk::SemaphoreCreateInfo semaphoreCreateInfo{};

		vk::FenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		mFrames.resize(frameCount);
		for (auto& [inFlight, finished] : mFrames)
		{
			inFlight = VkCore::Get()->Device().createFence(fenceCreateInfo);
			finished = VkCore::Get()->Device().createSemaphore(semaphoreCreateInfo);
		}
	}
}
