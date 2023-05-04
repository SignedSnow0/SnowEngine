#include "Image.h"

#include "Graphics/Vulkan/VkImage.h"

namespace SnowEngine
{
	std::shared_ptr<Image> Image::Create(const std::filesystem::path& source)
	{
		return std::make_shared<VkImage>(source);
	}
}
