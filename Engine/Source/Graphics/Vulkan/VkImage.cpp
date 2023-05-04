#include "VkImage.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "VkBuffers.h"
#include "VkCore.h"

namespace SnowEngine
{
	static stbi_uc* LoadImage(const std::filesystem::path& source, u32* width, u32* height)
	{
		i32 channels;
		stbi_uc* pixels{ stbi_load(source.string().c_str(), reinterpret_cast<i32*>(width), reinterpret_cast<i32*>(height), &channels, STBI_rgb_alpha) };

		return pixels;
	}

	VkImage::VkImage(const std::filesystem::path& source)
	{
		CreateImage(source);
		CreateView();
	}

	VkImage::~VkImage()
	{
		vmaDestroyImage(VkCore::Get()->Allocator(), mImage, mAllocation);
	}

	vk::ImageLayout VkImage::GetLayout() const { return  mLayout; }

	vk::ImageView VkImage::GetView() const { return mView; }

	void VkImage::CreateImage(const std::filesystem::path& source)
	{
		u32 width, height;
		auto* pixels{ LoadImage(source, &width, &height) };
		const VkBuffer staging{ width * height * 4, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_TO_GPU };
		staging.InsertData(pixels);
		stbi_image_free(pixels);

		mFormat = vk::Format::eR8G8B8A8Srgb;
		mLayout = vk::ImageLayout::eUndefined;

		vk::ImageCreateInfo createInfo{};
		createInfo.imageType = vk::ImageType::e2D;
		createInfo.extent.width = width;
		createInfo.extent.height = height;
		createInfo.extent.depth = 1;
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 1;
		createInfo.format = mFormat;
		createInfo.tiling = vk::ImageTiling::eOptimal;
		createInfo.initialLayout = mLayout;
		createInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		createInfo.sharingMode = vk::SharingMode::eExclusive;
		createInfo.samples = vk::SampleCountFlagBits::e1;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(VkCore::Get()->Allocator(), reinterpret_cast<VkImageCreateInfo*>(&createInfo), &allocInfo, reinterpret_cast<::VkImage*>(&mImage), &mAllocation, nullptr);

		ChangeLayout(vk::ImageLayout::eTransferDstOptimal);

		VkCore::Get()->SubmitInstantCommand([&](vk::CommandBuffer cmd)
		{
			vk::BufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = vk::Offset3D{ 0, 0, 0 };
			region.imageExtent = vk::Extent3D{ width, height, 1 };

			cmd.copyBufferToImage(staging.GetBuffer(), mImage, vk::ImageLayout::eTransferDstOptimal, region);
		});

		ChangeLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	void VkImage::CreateView()
	{
		vk::ImageViewCreateInfo createInfo{};
		createInfo.image = mImage;
		createInfo.viewType = vk::ImageViewType::e2D;
		createInfo.format = mFormat;
		createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		mView = VkCore::Get()->Device().createImageView(createInfo);
	}

	void VkImage::ChangeLayout(vk::ImageLayout newLayout)
	{
		vk::ImageMemoryBarrier barrier{};
		barrier.oldLayout = mLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = mImage;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vk::PipelineStageFlags sourceStage{};
		vk::PipelineStageFlags destinationStage{};

		if (mLayout == vk::ImageLayout::eUndefined)
		{
			barrier.srcAccessMask = {};
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		}
		else if (mLayout == vk::ImageLayout::eTransferDstOptimal) 
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			sourceStage = vk::PipelineStageFlagBits::eTransfer;
		}

		if (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		}
		else if (newLayout == vk::ImageLayout::eTransferDstOptimal) {
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;

			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		}
		else if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
			barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;

			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		}

		VkCore::Get()->SubmitInstantCommand([&](vk::CommandBuffer cmd) 
		{
			cmd.pipelineBarrier(sourceStage, destinationStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
		});

		mLayout = newLayout;
	}
}
