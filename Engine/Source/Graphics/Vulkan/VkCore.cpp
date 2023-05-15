#include "VkCore.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#include <set>
#include <GLFW/glfw3.h>
#include "Core/Types.h"
#include "VkValidationLayer.h"
#include "Core/Window.h"

namespace SnowEngine
{
	static vk::DebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo()
	{
		vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
		createInfo.pfnUserCallback = DebugCallback;
		createInfo.pUserData = nullptr;

		return createInfo;
	}

	VkCore* VkCore::sInstance{ nullptr };

	b8 VkQueues::IsComplete() const
	{
		return Graphics.first != UINT32_MAX
			&& Present.first != UINT32_MAX
			&& Compute.first != UINT32_MAX;
	}

	/**
	 * \brief Creates a new instance of VkCore if one does not already exist.
	 * \return Either the new instance or the existing instance.
	 */
	VkCore* VkCore::Create()
	{
		if (sInstance)
			return sInstance;

		sInstance = new VkCore();
		return sInstance;
	}

	VkCore::~VkCore()
	{
		vmaDestroyAllocator(mAllocator);

		DestroyDebugUtilsMessengerEXT(mInstance, mMessenger, nullptr);
	}

	const vk::Device& VkCore::Device() const { return mDevice; }

	const vk::PhysicalDevice& VkCore::PhysicalDevice() const { return mPhysicalDevice; }

	const vk::Instance& VkCore::Instance() const { return mInstance; }

	VkQueues VkCore::Queues() const { return mQueues; }

	VmaAllocator VkCore::Allocator() const { return mAllocator; }

	void VkCore::SubmitInstantCommand(std::function<void(vk::CommandBuffer cmd)>&& command) const
	{
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandPool = mInstantCommandPool;
		allocInfo.commandBufferCount = 1;

		static const vk::CommandBuffer commandBuffer{ mDevice.allocateCommandBuffers(allocInfo)[0] };

		const vk::CommandBufferBeginInfo beginInfo{};

		commandBuffer.reset();
		commandBuffer.begin(beginInfo);

		command(commandBuffer);

		commandBuffer.end();

		vk::SubmitInfo submitInfo{};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		mQueues.Graphics.second.submit(submitInfo, nullptr);
		mQueues.Graphics.second.waitIdle();
	}

	const VkCore* VkCore::Get() { return sInstance; }

	VkCore::VkCore()
	{
		CreateInstance();
		CreateDebugMessenger();
		CreatePhysicalDevice();
		CreateLogicalDevice();
		CreateAllocator();
		CreateInstantCommandPool();
	}

	void VkCore::CreateInstance()
	{
		vk::ApplicationInfo appInfo{};
		appInfo.pApplicationName = "SnowEditor";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "SnowEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		const std::vector<const char*> extensions{ GetRequiredExtensions() };
		const std::vector<const  char*> layers{ GetRequiredLayers() };

		const auto messengerCreateInfo{ GetDebugMessengerCreateInfo() };

		vk::InstanceCreateInfo createInfo{};
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.enabledLayerCount = static_cast<u32>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();
		createInfo.pNext = reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&messengerCreateInfo);

		mInstance = vk::createInstance(createInfo);
	}

	void VkCore::CreateDebugMessenger()
	{
		const auto createInfo{ GetDebugMessengerCreateInfo() };

		VkDebugUtilsMessengerEXT messenger;
		CreateDebugUtilsMessengerEXT(mInstance, reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), nullptr, &messenger);
		mMessenger = messenger;
	}

	void VkCore::CreatePhysicalDevice()
	{
		const std::vector<vk::PhysicalDevice> devices{ mInstance.enumeratePhysicalDevices() };

		for (const auto& device : devices)
		{
			if (auto [suitable, queues] = IsDeviceSuitable(device); suitable)
			{
				mPhysicalDevice = device;
				mQueues = queues;
				return;
			}
		}

		mPhysicalDevice = devices[0];
		mQueues = {};
	}

	void VkCore::CreateLogicalDevice()
	{
		std::set<u32> uniqueQueueFamilies
		{
			mQueues.Graphics.first,
			mQueues.Present.first,
			mQueues.Compute.first
		};

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{ uniqueQueueFamilies.size() };
		u32 i{ 0 };
		f32 queuePriority{ 1.0f };
		for (const auto& family : uniqueQueueFamilies)
		{
			vk::DeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.queueFamilyIndex = family;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos[i++] = queueCreateInfo;
		}

		vk::PhysicalDeviceFeatures enabledFeatures{};
		const auto enabledLayers{ GetRequiredLayers() };
		const auto enabledExtensions{ GetDeviceExtensions() };

		vk::DeviceCreateInfo createInfo{};
		createInfo.pEnabledFeatures = &enabledFeatures;
		createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.enabledLayerCount = static_cast<u32>(enabledLayers.size());
		createInfo.ppEnabledLayerNames = enabledLayers.data();
		createInfo.enabledExtensionCount = static_cast<u32>(enabledExtensions.size());
		createInfo.ppEnabledExtensionNames = enabledExtensions.data();

		mDevice = mPhysicalDevice.createDevice(createInfo);

		mQueues.Graphics.second = mDevice.getQueue(mQueues.Graphics.first, 0);
		mQueues.Present.second = mDevice.getQueue(mQueues.Present.first, 0);
		mQueues.Compute.second = mDevice.getQueue(mQueues.Compute.first, 0);
	}

	void VkCore::CreateAllocator()
	{
		VmaAllocatorCreateInfo createInfo{};
		createInfo.physicalDevice = mPhysicalDevice;
		createInfo.device = mDevice;
		createInfo.instance = mInstance;
		createInfo.vulkanApiVersion = VK_API_VERSION_1_2;

		vmaCreateAllocator(&createInfo, &mAllocator);
	}

	void VkCore::CreateInstantCommandPool()
	{
		vk::CommandPoolCreateInfo createInfo{};
		createInfo.queueFamilyIndex = mQueues.Graphics.first;
		createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		mInstantCommandPool = mDevice.createCommandPool(createInfo);
	}

	std::pair<b8, VkQueues> VkCore::IsDeviceSuitable(const vk::PhysicalDevice& device) const
	{
		b8 suitable{ true };
		VkQueues queues{};

		const auto window = Window::Create("Query window", 100, 100, false, false, false);
		VkSurfaceKHR surface;
		glfwCreateWindowSurface(mInstance, window->Handle(), nullptr, &surface);

		const std::vector<vk::QueueFamilyProperties> queueFamilies{ device.getQueueFamilyProperties() };
		u32 i{ 0 };
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
				queues.Graphics.first = i;

			if (device.getSurfaceSupportKHR(i, surface))
				queues.Present.first = i;

			if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)
				queues.Compute.first = i;

			if (queues.IsComplete())
				break;

			i++;
		}

		const vk::PhysicalDeviceProperties properties{ device.getProperties() };
		const vk::PhysicalDeviceFeatures features{ device.getFeatures() };

		suitable &= CheckExtensionSupport(device, GetDeviceExtensions());
		suitable &= queues.IsComplete();

		return { suitable, queues };
	}

	b8 VkCore::CheckExtensionSupport(const vk::PhysicalDevice& device, const std::vector<const char*>& extensions)
	{
		std::set<std::string> requiredExtensions{ extensions.begin(), extensions.end() };
		for (const auto& extension : device.enumerateDeviceExtensionProperties())
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	std::vector<const char*> VkCore::GetDeviceExtensions()
	{
		std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		return extensions;
	}

	std::vector<const char*> VkCore::GetRequiredExtensions()
	{
		glfwInit();//TODO: ugly

		u32 glfwExtensionCount{ 0 };
		const char** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

		std::vector<const char*> extensions{ glfwExtensions, glfwExtensions + glfwExtensionCount };
		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);//TODO: vk debug conf

		return extensions;
	}

	std::vector<const char*> VkCore::GetRequiredLayers()
	{
		std::vector<const char*> layers{};

		layers.emplace_back("VK_LAYER_KHRONOS_validation");//TODO: vk debug conf

		const auto availableLayers{ vk::enumerateInstanceLayerProperties() };

		u32 missingLayers{ static_cast<u32>(layers.size()) };
		for (const auto* layerName : layers)
		{
			for (const auto& properties : availableLayers)
			{
				if (strcmp(layerName, properties.layerName) == 0)
				{
					missingLayers--;
				}

			}
		}

		return missingLayers != 0 ? std::vector<const char*>{} : layers;
	}
}
