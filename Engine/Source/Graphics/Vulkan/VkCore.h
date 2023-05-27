#pragma once
#include <functional>
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#include "Core/Types.h"
#include "Graphics/Rhi/Core.h"

namespace SnowEngine
{
	struct VkQueues
	{
		std::pair<u32, vk::Queue> Graphics = { UINT32_MAX, nullptr };
		std::pair<u32, vk::Queue> Present = { UINT32_MAX, nullptr };
		std::pair<u32, vk::Queue> Compute = { UINT32_MAX, nullptr };

		b8 IsComplete() const;
	};

	class VkCore : public GraphicsCore
	{
	public:
		static VkCore* Create();
		~VkCore() override;

		const vk::Device& Device() const;
		const vk::PhysicalDevice& PhysicalDevice() const;
		const vk::Instance& Instance() const;
		VkQueues Queues() const;
		VmaAllocator Allocator() const;

		void DeviceWaitIdle() const override;
		void SubmitInstantCommand(std::function<void(vk::CommandBuffer cmd)>&& command) const;

		static const VkCore* Get();

	private:
		VkCore();
		void CreateInstance();
		void CreateDebugMessenger();
		void CreatePhysicalDevice();
		void CreateLogicalDevice();
		void CreateAllocator();
		void CreateInstantCommandPool();

		std::pair<b8, VkQueues> IsDeviceSuitable(const vk::PhysicalDevice& device) const;
		static b8 CheckExtensionSupport(const vk::PhysicalDevice& device, const std::vector<const char*>& extensions);
		static std::vector<const char*> GetDeviceExtensions();
		static std::vector<const char*> GetRequiredExtensions();
		static std::vector<const char*> GetRequiredLayers();
		
		vk::Instance mInstance;
		vk::DebugUtilsMessengerEXT mMessenger;
		vk::PhysicalDevice mPhysicalDevice;
		vk::Device mDevice;
		VkQueues mQueues;
		VmaAllocator mAllocator;
		vk::CommandPool mInstantCommandPool;
		static VkCore* sInstance;
	};
}