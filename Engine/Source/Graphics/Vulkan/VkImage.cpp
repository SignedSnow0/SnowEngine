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
		CreateImage({ source });
		CreateView(vk::ImageAspectFlagBits::eColor);
	}

	VkImage::VkImage(const std::array<std::filesystem::path, 6>& sources)
	{
		CreateImage({ sources.begin(), sources.end() });
		CreateView(vk::ImageAspectFlagBits::eColor, static_cast<u32>(sources.size()));
	}

	VkImage::VkImage(const u32 width, const u32 height, const vk::Format format, const vk::ImageUsageFlags usage, const vk::ImageLayout layout, const vk::ImageAspectFlags aspect)
		: mFormat{ format }
	{
		CreateImage(width, height, usage, layout);
		CreateView(aspect);
	}

	VkImage::~VkImage()
	{
		vmaDestroyImage(VkCore::Get()->Allocator(), mImage, mAllocation);
	}

	vk::ImageLayout VkImage::Layout() const { return mLayout; }

	vk::ImageView VkImage::View() const { return mView; }

	void VkImage::CreateImage(const u32 width, const u32 height, const vk::ImageUsageFlags usage, const vk::ImageLayout layout, const u32 arrayLayers)
	{
		vk::ImageCreateInfo createInfo{};
		createInfo.imageType = vk::ImageType::e2D;
		createInfo.extent.width = width;
		createInfo.extent.height = height;
		createInfo.extent.depth = 1;
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = arrayLayers;
		createInfo.format = mFormat;
		createInfo.tiling = vk::ImageTiling::eOptimal;
		createInfo.initialLayout = mLayout;
		createInfo.usage = usage;
		createInfo.sharingMode = vk::SharingMode::eExclusive;
		createInfo.samples = vk::SampleCountFlagBits::e1;

		if (arrayLayers == 6) //TODO: argument
			createInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		auto res = vmaCreateImage(VkCore::Get()->Allocator(), reinterpret_cast<VkImageCreateInfo*>(&createInfo), &allocInfo, reinterpret_cast<::VkImage*>(&mImage), &mAllocation, nullptr);

		ChangeLayout(layout, arrayLayers);
	}

	void VkImage::CreateImage(const std::vector<std::filesystem::path>& sources)
	{
		std::vector<VkBuffer> buffers;
		buffers.reserve(sources.size());

		u32 width, height;
		for (u32 i{ 0 }; i < sources.size(); i++)
		{
			auto* pixels{ LoadImage(sources[i], &width, &height)};

			buffers.emplace_back(width * height * 4, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_TO_GPU);
			buffers.back().InsertData(pixels);

			stbi_image_free(pixels);
		}

		mFormat = vk::Format::eR8G8B8A8Srgb;
		CreateImage(width, height, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageLayout::eTransferDstOptimal, sources.size());

		VkCore::Get()->SubmitInstantCommand([&](const vk::CommandBuffer cmd)
		{
			for (u32 i{ 0 }; i < sources.size(); i++)
			{
				vk::BufferImageCopy region{};
				region.bufferOffset = 0;
				region.bufferRowLength = 0;
				region.bufferImageHeight = 0;
				region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				region.imageSubresource.mipLevel = 0;
				region.imageSubresource.baseArrayLayer = i;
				region.imageSubresource.layerCount = 1;
				region.imageOffset = vk::Offset3D{ 0, 0, 0 };
				region.imageExtent = vk::Extent3D{ width, height, 1 };

				cmd.copyBufferToImage(buffers[i].Buffer(), mImage, vk::ImageLayout::eTransferDstOptimal, region);
			}
		});

		ChangeLayout(vk::ImageLayout::eShaderReadOnlyOptimal, sources.size());
	}

	void VkImage::CreateView(const vk::ImageAspectFlags aspect, const u32 arrayLayers)
	{
		vk::ImageViewCreateInfo createInfo{};
		createInfo.image = mImage;
		createInfo.viewType = arrayLayers == 6 ? vk::ImageViewType::eCube : vk::ImageViewType::e2D;//TODO: argument
		createInfo.format = mFormat;
		createInfo.subresourceRange.aspectMask = aspect;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = arrayLayers;

		mView = VkCore::Get()->Device().createImageView(createInfo);
	}

	void VkImage::ChangeLayout(const vk::ImageLayout newLayout, const u32 arrayLayers)
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
		barrier.subresourceRange.layerCount = arrayLayers;

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

		VkCore::Get()->SubmitInstantCommand([&](const vk::CommandBuffer cmd) 
		{
			cmd.pipelineBarrier(sourceStage, destinationStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
		});

		mLayout = newLayout;
	}
}
