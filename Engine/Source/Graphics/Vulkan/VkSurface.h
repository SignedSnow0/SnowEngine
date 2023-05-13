#pragma once
#include <functional>
#include <vulkan/vulkan.hpp>
#include "Graphics/Rhi/Surface.h"
#include "Core/Window.h"

namespace SnowEngine
{
	class VkSurface : public Surface
	{
	public:
		VkSurface(std::shared_ptr<const Window> window);

		u32 ImageCount() const override;
		u32 CurrentFrame() const override;

		u32 Width() const;
		u32 Height() const;
		vk::Format Format() const;
		std::vector<vk::ImageView> Views() const;
		std::shared_ptr<const Window> GetWindow() const;
		vk::Semaphore ImageAvailableSemaphore() const;

		void Begin() override;
		void End(const std::shared_ptr<const CommandBuffer>& commandBuffer) override;

		void SubmitPostFrameQueue(const std::function<void(u32 frameIndex)>& func);

		static VkSurface* BoundSurface();

	private:
		void CreateSurface();
		void CreateSurfaceSettings();
		void CreateSwapchain();
		void CreateFrameData();
		void CreateSyncObjects();
		void FlushPostSubmitQueue();
		void Resize();

		struct FrameData
		{
			vk::Image Image;
			vk::ImageView ImageView;
			vk::Semaphore ImageAvailable;
		};

		std::vector<FrameData> mFrames;
		vk::SurfaceKHR mSurface;
		vk::PresentModeKHR mPresentMode;
		vk::SurfaceFormatKHR mSurfaceFormat;
		vk::Extent2D mExtent;
		u32 mImageCount;
		u32 mCurrentPresentFrame{ 0 };
		u32 mCurrentCpuFrame;
		vk::SwapchainKHR mSwapchain;
		std::shared_ptr<const Window> mWindow;
		std::vector<std::pair<u32, std::function<void(u32 frameIndex)>>> mPostSubmitQueue;

		static VkSurface* sBoundSurface;
	};
}
