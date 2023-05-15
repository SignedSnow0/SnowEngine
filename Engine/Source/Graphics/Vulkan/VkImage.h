#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include <filesystem>

#include "Core/Types.h"
#include "Graphics/Rhi/Image.h"

namespace SnowEngine
{
	class VkImage : public Image
	{
	public:
		VkImage(const std::filesystem::path& source);
		VkImage(u32 width, u32 height, vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout layout, vk::ImageAspectFlags aspect);
		~VkImage() override;

		vk::ImageLayout Layout() const;
		vk::ImageView View() const;

	private:
		void CreateImage(u32 width, u32 height, vk::ImageUsageFlags usage, vk::ImageLayout layout);
		void CreateImage(const std::filesystem::path& source);
		void CreateView(vk::ImageAspectFlags aspect);
		void ChangeLayout(vk::ImageLayout newLayout);

		vk::ImageLayout mLayout{ vk::ImageLayout::eUndefined };
		vk::Format mFormat;
		vk::Image mImage;
		vk::ImageView mView;
		VmaAllocation mAllocation;
	};
}
