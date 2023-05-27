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
		VkImage(const std::array<std::filesystem::path, 6>& sources);
		VkImage(u32 width, u32 height, vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout layout, vk::ImageAspectFlags aspect);
		~VkImage() override;

		vk::ImageLayout Layout() const;
		vk::ImageView View() const;

	private:
		void CreateImage(u32 width, u32 height, vk::ImageUsageFlags usage, vk::ImageLayout layout, u32 arrayLayers = 1);
		void CreateImage(const std::vector<std::filesystem::path>& sources);
		void CreateView(vk::ImageAspectFlags aspect, u32 arrayLayers = 1);
		void ChangeLayout(vk::ImageLayout newLayout, u32 arrayLayers);

		vk::ImageLayout mLayout{ vk::ImageLayout::eUndefined };
		vk::Format mFormat;
		vk::Image mImage;
		vk::ImageView mView;
		VmaAllocation mAllocation;
	};
}
