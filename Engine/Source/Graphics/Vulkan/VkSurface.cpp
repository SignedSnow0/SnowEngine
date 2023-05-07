#include "VkSurface.h"

#include "VkCore.h"

namespace SnowEngine
{
	VkSurface* VkSurface::sBoundSurface{ nullptr };

	VkSurface::VkSurface(std::shared_ptr<const Window> window)
		: mWindow{std::move(window) }
	{
		CreateSurface();
		CreateSurfaceSettings();
		CreateSwapchain();
		CreateCommandPool();
		CreateFrameData();
	}

	vk::Format VkSurface::GetFormat() const { return mSurfaceFormat.format; }

	std::vector<vk::ImageView> VkSurface::GetViews() const
	{
		std::vector<vk::ImageView> views;
		views.reserve(mFrames.size());
		for (const auto& frame : mFrames)
			views.push_back(frame.ImageView);

		return views;
	}

	u32 VkSurface::GetWidth() const { return mExtent.width; }

	u32 VkSurface::GetHeight() const { return mExtent.height; }

	std::shared_ptr<const Window> VkSurface::GetWindow() const { return mWindow; }

	u32 VkSurface::GetCurrentFrame() const { return mCurrentCpuFrame; }

	vk::CommandBuffer VkSurface::GetCommandBuffer() const { return mFrames[mCurrentPresentFrame].CommandBuffer; }

	void VkSurface::Begin()
	{
		sBoundSurface = this;
		
		vk::Result result = VkCore::Get()->Device().waitForFences(mFrames[mCurrentPresentFrame].InFlight, true, std::numeric_limits<u64>::max());

		try
		{
			result = VkCore::Get()->Device().acquireNextImageKHR(mSwapchain, std::numeric_limits<u64>::max(), mFrames[mCurrentPresentFrame].ImageAvailable, nullptr, &mCurrentCpuFrame);
		}
		catch (const vk::OutOfDateKHRError&)
		{
			
		}

		FlushPostSubmitQueue();

		VkCore::Get()->Device().resetFences(mFrames[mCurrentPresentFrame].InFlight);

		mFrames[mCurrentPresentFrame].CommandBuffer.reset();
		mFrames[mCurrentPresentFrame].CommandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	}

	void VkSurface::End()
	{
		mFrames[mCurrentPresentFrame].CommandBuffer.end();

		const vk::Semaphore waitSemaphores[] = { mFrames[mCurrentPresentFrame].ImageAvailable };
		const vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		vk::SubmitInfo submitInfo;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mFrames[mCurrentPresentFrame].CommandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &mFrames[mCurrentPresentFrame].RenderFinished;

		VkCore::Get()->Queues()[0].Queue.submit(submitInfo, mFrames[mCurrentPresentFrame].InFlight);//TODO: queue index

		vk::PresentInfoKHR presentInfo;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &mFrames[mCurrentPresentFrame].RenderFinished;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &mSwapchain;
		presentInfo.pImageIndices = &mCurrentCpuFrame;

		VkCore::Get()->Queues()[1].Queue.presentKHR(presentInfo);

		mCurrentPresentFrame = (mCurrentPresentFrame + 1) % mFrames.size() - 1;

		sBoundSurface = nullptr;
	}

	void VkSurface::SubmitPostFrameQueue(const std::function<void(u32 frameIndex)>& func)
	{
		mPostSubmitQueue.emplace_back(0, func);
	}

	VkSurface* VkSurface::BoundSurface() { return sBoundSurface; }

	void VkSurface::CreateSurface()
	{
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(VkCore::Get()->Instance(), mWindow->Handle(), nullptr, &surface) == VK_SUCCESS)
		{
			mSurface = surface;
			return;
		}

		mSurface = nullptr;
	}

	void VkSurface::CreateSurfaceSettings()
	{
		const auto& physicalDevice = VkCore::Get()->PhysicalDevice();
		const vk::SurfaceCapabilitiesKHR capabilities{ physicalDevice.getSurfaceCapabilitiesKHR(mSurface) };
		const std::vector<vk::SurfaceFormatKHR> formats{ physicalDevice.getSurfaceFormatsKHR(mSurface) };
		const std::vector<vk::PresentModeKHR> presentModes{ physicalDevice.getSurfacePresentModesKHR(mSurface) };

		mSurfaceFormat = formats[0];
		for (const auto& format : formats)
		{
			if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				mSurfaceFormat = format;
				break;
			}
		}

		mPresentMode = vk::PresentModeKHR::eFifo;
		for (const auto& presentMode : presentModes)
		{
			if (presentMode == vk::PresentModeKHR::eMailbox)
			{
				mPresentMode = presentMode;
				break;
			}
		}

		if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
		{
			mExtent = capabilities.currentExtent;
		}
		else
		{
			mExtent = vk::Extent2D{ mWindow->Width(), mWindow->Height() };
			mExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, mExtent.width));
			mExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, mExtent.height));
		}

		mImageCount = capabilities.minImageCount;
	}

	void VkSurface::CreateSwapchain()
	{
		const std::vector<u32> qeues = { VkCore::Get()->Queues()[0].Family, VkCore::Get()->Queues()[1].Family };
		const b8 sharedQueue{ qeues[0] == qeues[1] };

		vk::SwapchainCreateInfoKHR createInfo{};
		createInfo.surface = mSurface;
		createInfo.minImageCount = mImageCount;
		createInfo.imageFormat = mSurfaceFormat.format;
		createInfo.imageColorSpace = mSurfaceFormat.colorSpace;
		createInfo.imageExtent = mExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
		createInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = mPresentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = nullptr;
		createInfo.imageSharingMode = sharedQueue ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = sharedQueue ? 0 : 2;
		createInfo.pQueueFamilyIndices = sharedQueue ? nullptr : qeues.data();

		mSwapchain = VkCore::Get()->Device().createSwapchainKHR(createInfo);
	}

	void VkSurface::CreateCommandPool()
	{
		vk::CommandPoolCreateInfo createInfo{};
		createInfo.queueFamilyIndex = VkCore::Get()->Queues()[0].Family;
		createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		mCommandPool = VkCore::Get()->Device().createCommandPool(createInfo);
	}

	void VkSurface::CreateFrameData()
	{
		mFrames.resize(mImageCount);
		const auto images{ VkCore::Get()->Device().getSwapchainImagesKHR(mSwapchain) };

		vk::ImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.viewType = vk::ImageViewType::e2D;
		viewCreateInfo.format = mSurfaceFormat.format;
		viewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
		viewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
		viewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
		viewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
		viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		vk::SemaphoreCreateInfo semaphoreCreateInfo{};

		vk::FenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		u32 i{ 0 };
		for (auto& [image, imageView, commandBuffer, imageAvailable, renderFinished, inFlight] : mFrames)
		{
			image = images[i];

			viewCreateInfo.image = image;
			imageView = VkCore::Get()->Device().createImageView(viewCreateInfo);

			vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
			commandBufferAllocateInfo.commandPool = mCommandPool;
			commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
			commandBufferAllocateInfo.commandBufferCount = 1;

			commandBuffer = VkCore::Get()->Device().allocateCommandBuffers(commandBufferAllocateInfo)[0];

			imageAvailable = VkCore::Get()->Device().createSemaphore(semaphoreCreateInfo);
			renderFinished = VkCore::Get()->Device().createSemaphore(semaphoreCreateInfo);
			inFlight = VkCore::Get()->Device().createFence(fenceCreateInfo);

			i++;
		}
	}

	void VkSurface::FlushPostSubmitQueue()
	{
		for (u32 i{ 0 }; i < mPostSubmitQueue.size(); i++) {
			auto& [count, func] = mPostSubmitQueue[i];
			func(mCurrentCpuFrame);
			count++;
			if (count == mImageCount) {
				mPostSubmitQueue.erase(mPostSubmitQueue.begin() + i);
				i--;
			}
		}
	}
}
