#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include <filesystem>
#include "Graphics/Rhi/Image.h"

namespace SnowEngine
{
	class VkImage : public Image
	{
	public:
		VkImage(const std::filesystem::path& source);
		~VkImage() override;

		vk::ImageLayout GetLayout() const;
		vk::ImageView GetView() const;

	private:
		void CreateImage(const std::filesystem::path& source);
		void CreateView();
		void ChangeLayout(vk::ImageLayout newLayout);

		vk::ImageLayout mLayout;
		vk::Format mFormat;
		vk::Image mImage;
		vk::ImageView mView;
		VmaAllocation mAllocation;
	};
}
