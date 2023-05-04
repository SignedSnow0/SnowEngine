#include "Surface.h"

#include "Graphics/Vulkan/VkSurface.h"

namespace SnowEngine
{
	std::shared_ptr<Surface> Surface::Create(std::shared_ptr<const Window> window)
	{
		return std::make_shared<VkSurface>(window);
	}
}
