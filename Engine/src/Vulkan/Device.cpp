#include "Device.h"
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <set>
#include <array>

namespace SnowEngine
{
	Device* Device::currentDevice;

    static const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	//estensioni a livello device
	static const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME
	};

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		//visto che la funzione è di un estensione non viene caricata automaticamente e dobbiamo usare questa funzione per trovarne il puntatore
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger); //se la funzione esiste la eseguo e ne restituisco il risultato per controllare eventuali errori
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	//                                                       importanza del messaggio         |   indica se il messaggio è un errore o uso non ottimale | messaggio + oggetti chiave + numero oggetti chiave
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "[Validation layer]: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE; //indica se abortire o meno la funzione che ha causato l`errore
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
			func(instance, debugMessenger, pAllocator);
	}

	Device::Device(Window& window) : window(window)
	{
		InitInstance();
		SetupValidationCallbacks();
		CreateSurface();
		SelectPhysicalDevice();
		CreateLogicalDevice();
		CreateCommandPool();
		CreateDescriptorPool();

		currentDevice = this;
		
		LoadExtensionFunctions();
		CreateAllocator();
	}

	Device::~Device()
	{
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorPool(device, imguiPool, nullptr);

		vkDestroyCommandPool(device, commandPool, nullptr);

		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers)
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

		vkDestroySurfaceKHR(instance, surface, nullptr);

		vkDestroyInstance(instance, nullptr);
	}

	VkCommandBuffer Device::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool			= commandPool;
		allocInfo.commandBufferCount	= 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}

	void Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags bufferType, VkMemoryPropertyFlags properties, VkBuffer* buffer, VmaAllocation* allocation)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = bufferType;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.requiredFlags = properties;
		if (bufferType == VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			allocInfo.usage = VMA_MEMORY_USAGE_CPU_COPY;
		else if (VK_BUFFER_USAGE_TRANSFER_DST_BIT)
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, buffer, allocation, nullptr);
	}

	void Device::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VmaAllocation* allocation, VkImageType type, VkImageCreateFlags flags)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType		= type;
		imageInfo.extent.width	= width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth	= 1;
		imageInfo.mipLevels		= 1;
		imageInfo.arrayLayers	= 1;	
		imageInfo.format		= format;
		imageInfo.tiling		= tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage			= usage;
		imageInfo.samples		= VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags			= flags;
		
		if (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
			imageInfo.arrayLayers = 6;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.requiredFlags = properties;

		vmaCreateImage(allocator, &imageInfo, &allocInfo, image, allocation, nullptr);
	}

	VkImageView Device::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) 
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image								= image;
		viewInfo.viewType							= VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format								= format;
		viewInfo.subresourceRange.aspectMask		= aspectFlags;
		viewInfo.subresourceRange.baseMipLevel		= 0;
		viewInfo.subresourceRange.levelCount		= 1;
		viewInfo.subresourceRange.baseArrayLayer	= 0;
		viewInfo.subresourceRange.layerCount		= 1;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
			throw std::runtime_error("Failed to create texture image view!");

		return imageView;
	}

	void Device::EndSingleTimeCommands(VkCommandBuffer commandBuffer) 
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	VkFormat Device::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				return format;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				return format;
		}

		throw std::runtime_error("Failed to find supported format!");
	}

	bool Device::CheckDeviceExtensionsSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		return requiredExtensions.empty();
	}

	bool Device::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) { //controlla se tutti i layer richiesti (validationLayers) sono disponibili (availableLayers)
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
				if (strcmp(layerName, layerProperties.layerName) == 0)
					layerFound = true;
			break;

			if (!layerFound)
				return false;
		}

		return true;
	}

	void Device::CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex	= queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
			throw std::runtime_error("failed to create command pool!");
	}

	void Device::CreateDescriptorPool() 
	{
		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0].type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount	= 1000;
		poolSizes[1].type				= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount	= 1000;
		poolSizes[2].type				= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[2].descriptorCount	= 1000;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount	= static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes		= poolSizes.data();
		poolInfo.maxSets		= 1000;
		poolInfo.flags			= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor pool!");

		std::vector<VkDescriptorPoolSize> imguiSizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(imguiSizes.size());
		poolInfo.pPoolSizes = imguiSizes.data();
		poolInfo.maxSets = 1;

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &imguiPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor pool!");
	}

	void Device::CreateLogicalDevice() 
	{
		float queuePriority	= 1.0f; //ordine di esecuzione di più queue dello stesso tipo 0.0f - 1.0f
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{}; //creiamo delle queue dai tipi disponibili nel physicaldevice
			queueCreateInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex	= indices.graphicsFamily.value();
			queueCreateInfo.queueCount			= 1;
			queueCreateInfo.pQueuePriorities	= &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{}; //features optionali che vogliamo usare nel programma (devono essere presenti nel physical device)
		deviceFeatures.samplerAnisotropy = VK_TRUE; //usato nel texture samplers

		//////////////////////////////////////////////////////External features////////////////////////////////////////////////////////////////////////////////
		VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedFeatures{};
		extendedFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
		extendedFeatures.extendedDynamicState = VK_TRUE;
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		VkDeviceCreateInfo createInfo{};
		createInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos		= queueCreateInfos.data();
		createInfo.queueCreateInfoCount		= static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures			= &deviceFeatures;
		createInfo.enabledExtensionCount	= static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames	= deviceExtensions.data();
		createInfo.pNext = &extendedFeatures;
		
		if (enableValidationLayers) { //parte non necessaria: ora le estensioni di validation non sono supportate a livello device
			createInfo.enabledLayerCount	= static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames	= validationLayers.data();
		}
		else
			createInfo.enabledLayerCount	= 0;

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device!");

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	void Device::CreateSurface() {
		window.CreateWindowSurface(instance, &surface);
	}

	void Device::FillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo					= {};
		createInfo.sType			= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback	= DebugCallback;
		createInfo.pUserData		= nullptr; // Optional
	}

	uint32_t Device::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) //buffer con tipo di memoria adatto e disponibile per la scrittura
				return i;

		throw std::runtime_error("Failed to find suitable memory type!");
	}

	Device::QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice device) 
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0; //otteniamo tutte le famiglie disponibili per il device
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		VkBool32 presentSupport = false;

		int i = 0;
		for (const auto& queueFamily : queueFamilies) { //controlliamo se ogni famiglia richiesta è presente nel device
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphicsFamily = i; //se è presente le diamo un valore perchè abbiamo usato std::optional

			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
				indices.presentFamily = i;

			if (indices.IsComplete())
				break;

			i++;
		}

		return indices;
	}

	void Device::GetAvailableExtensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "Available instance extensions:\n";
		for (const auto& extension : extensions)
			std::cout << '\t' << extension.extensionName << '\n';
	}

	std::vector<const char*> Device::GetRequiredExtension()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	void Device::InitInstance()
	{
		VkApplicationInfo appInfo{};
		appInfo.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName	= "No name";
		appInfo.applicationVersion	= VK_MAKE_VERSION(0, 1, 0);
		appInfo.pEngineName			= "SnowEngine";
		appInfo.engineVersion		= VK_MAKE_VERSION(0, 1, 0);
		appInfo.apiVersion			= VK_MAKE_VERSION(1, 0, 0); //https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_API_VERSION.html

		std::vector<const char*> extensions = GetRequiredExtension();
		if (enableValidationLayers && !CheckValidationLayerSupport()) //controlla se abbiamo bisogno dei layer e se sono disponibili
			throw std::runtime_error("Validation layers requested, but not available!");
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType					= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo			= &appInfo;
		createInfo.enabledExtensionCount	= static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames	= extensions.data();

		if (enableValidationLayers) {
			FillDebugMessengerCreateInfo(debugCreateInfo);

			createInfo.enabledLayerCount	= static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames	= validationLayers.data();
			createInfo.pNext				= (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo; //inserendo in pnext la creazione dei validation layer è possibile debuggare la creazione dell`istanza prima che i validation layer vengano creati
		}
		else {
			createInfo.enabledLayerCount	= 0;
			createInfo.pNext				= nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create instance!");

		GetAvailableExtensions();
	}

	bool Device::IsDeviceSuitable(VkPhysicalDevice device) 
	{
		QueueFamilyIndices indices = FindQueueFamilies(device);

		bool extensionsSupported = CheckDeviceExtensionsSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	SwapChainSupportDetails Device::QuerySwapChainSupport(VkPhysicalDevice device) 
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities); //ottengo le capabilities

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data()); //ottengo tutti i formati
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data()); //ottengo tutte le modalità di presentazione
		}

		return details;
	}

	void SnowEngine::Device::SelectPhysicalDevice() 
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0)
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices)
			if (IsDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}

		if (physicalDevice == VK_NULL_HANDLE){
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
		uint32_t extensionsCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, availableExtensions.data());

		std::cout << "Available device extensions:\n";
		for (const auto& extension : availableExtensions)
			std::cout << '\t' << extension.extensionName << '\n';
	}

	void Device::SetupValidationCallbacks()
	{
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		FillDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) //chiamo la funzione per trovare la funzione e poi eseguirla
			throw std::runtime_error("Failed to set up debug messenger!");
	}

	void Device::LoadExtensionFunctions() 
	{
		vkCmdSetCullMode = (PFN_vkCmdSetCullModeEXT)vkGetInstanceProcAddr(instance, "vkCmdSetCullModeEXT");
	}

	void Device::CreateAllocator()
	{
		VmaAllocatorCreateInfo createInfo{};
		createInfo.vulkanApiVersion = VK_API_VERSION_1_0;
		createInfo.physicalDevice	= physicalDevice;
		createInfo.device			= device;
		createInfo.instance			= instance;

		vmaCreateAllocator(&createInfo, &allocator);
	}
}