#pragma once
#include <vulkan/vulkan.hpp>
#include "Graphics/Rhi/Surface.h"
#include "Core/Window.h"

namespace SnowEngine
{
	class VkSurface : public Surface
	{
	public:
		VkSurface(std::shared_ptr<const Window> window);

		vk::Format GetFormat() const;
		std::vector<vk::ImageView> GetViews() const;
		u32 GetWidth() const;
		u32 GetHeight() const;
		vk::CommandBuffer GetCommandBuffer() const;
		u32 GetCurrentFrame() const override;

		void Begin() override;
		void End() override;

		static const VkSurface* BoundSurface();

	private:
		void CreateSurface();
		void CreateSurfaceSettings();
		void CreateSwapchain();
		void CreateCommandPool();
		void CreateFrameData();

		struct FrameData
		{
			vk::Image Image;
			vk::ImageView ImageView;
			vk::CommandBuffer CommandBuffer;
			vk::Semaphore ImageAvailable;
			vk::Semaphore RenderFinished;
			vk::Fence InFlight;
		};

		std::vector<FrameData> mFrames;
		vk::SurfaceKHR mSurface;
		vk::PresentModeKHR mPresentMode;
		vk::SurfaceFormatKHR mSurfaceFormat;
		vk::Extent2D mExtent;
		u32 mImageCount;
		u32 mCurrentPresentFrame{ 0 };
		u32 mCurrentCpuFrame;
		vk::CommandPool mCommandPool;
		vk::SwapchainKHR mSwapchain;
		std::shared_ptr<const Window> mWindow;

		static const VkSurface* sBoundSurface;
	};
}
