#include "Core.h"
#include "Graphics/Vulkan/VkCore.h"

namespace SnowEngine
{
	GraphicsCore* GraphicsCore::sInstance{ nullptr };

	GraphicsCore* GraphicsCore::Init()
	{
		if (!sInstance)
		{
			sInstance =  VkCore::Create();
		}

		return sInstance;
	}

	void GraphicsCore::Shutdown()
	{
		if (sInstance)
		{
			delete sInstance;
			sInstance = nullptr;
		}
	}
}