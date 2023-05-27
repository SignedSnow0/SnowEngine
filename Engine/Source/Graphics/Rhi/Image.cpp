#include "Image.h"

#include "Graphics/Vulkan/VkImage.h"

namespace SnowEngine
{
	std::shared_ptr<Image> Image::Create(const std::filesystem::path& source)
	{
		return std::make_shared<VkImage>(source);
	}

	std::shared_ptr<Image> Image::Create(const std::array<std::filesystem::path, 6>& sources)
	{
		return std::make_shared<VkImage>(sources);
	}
}
