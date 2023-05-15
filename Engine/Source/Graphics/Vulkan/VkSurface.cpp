#include "VkSurface.h"

#include "VkCore.h"
#include "VkCommandBuffer.h"

namespace SnowEngine
{
	VkSurface* VkSurface::sBoundSurface{ nullptr };

	VkSurface::VkSurface(std::shared_ptr<const Window> window)
		: mWindow{std::move(window) }
	{
		CreateSurface();
		CreateSurfaceSettings();
		CreateSwapchain();
		CreateFrameData();
		CreateSyncObjects();
	}

	u32 VkSurface::ImageCount() const { return mImageCount; }

	vk::Format VkSurface::Format() const { return mSurfaceFormat.format; }

	std::vector<vk::ImageView> VkSurface::Views() const
	{
		std::vector<vk::ImageView> views;
		views.reserve(mFrames.size());
		for (const auto& frame : mFrames)
			views.push_back(frame.ImageView);

		return views;
	}

	u32 VkSurface::Width() const { return mExtent.width; }

	u32 VkSurface::Height() const { return mExtent.height; }

	std::shared_ptr<const Window> VkSurface::GetWindow() const { return mWindow; }

	vk::Semaphore VkSurface::ImageAvailableSemaphore() const { return mFrames[mCurrentPresentFrame].ImageAvailable; }

	u32 VkSurface::CurrentFrame() const { return mCurrentCpuFrame; }

	void VkSurface::Begin()
	{
		sBoundSurface = this;
		
		try
		{
			vk::Result result = VkCore::Get()->Device().acquireNextImageKHR(mSwapchain, std::numeric_limits<u64>::max(), mFrames[mCurrentPresentFrame].ImageAvailable, nullptr, &mCurrentCpuFrame);
		}
		catch (const vk::OutOfDateKHRError&)
		{
			Resize();
		}

		FlushPostSubmitQueue();
	}

	void VkSurface::End(const std::shared_ptr<const CommandBuffer>& commandBuffer)
	{
		const auto waitSemaphore = std::static_pointer_cast<const VkCommandBuffer>(commandBuffer)->FinishedSemaphore(mCurrentCpuFrame);

		vk::PresentInfoKHR presentInfo;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &waitSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &mSwapchain;
		presentInfo.pImageIndices = &mCurrentCpuFrame;

		const vk::Result result = VkCore::Get()->Queues().Present.second.presentKHR(presentInfo);
		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
			Resize();

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
			if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
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

		mImageCount = capabilities.minImageCount;
	}

	void VkSurface::CreateSwapchain()
	{
		const vk::SurfaceCapabilitiesKHR capabilities{ VkCore::Get()->PhysicalDevice().getSurfaceCapabilitiesKHR(mSurface)};
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

		const std::vector<u32> qeues = { VkCore::Get()->Queues().Graphics.first, VkCore::Get()->Queues().Present.first };
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

		u32 i{ 0 };
		for (auto& frame : mFrames)
		{
			frame.Image = images[i];

			viewCreateInfo.image = frame.Image;
			frame.ImageView = VkCore::Get()->Device().createImageView(viewCreateInfo);

			i++;
		}
	}

	void VkSurface::CreateSyncObjects()
	{
		const vk::SemaphoreCreateInfo semaphoreCreateInfo{};

		for (auto& [im, v, imageAvailable] : mFrames)
			imageAvailable = VkCore::Get()->Device().createSemaphore(semaphoreCreateInfo);
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

	void VkSurface::Resize()
	{
		VkCore::Get()->Device().waitIdle();

		VkCore::Get()->Device().destroySwapchainKHR(mSwapchain);

		CreateSwapchain();
		CreateFrameData();
	}
}
