#pragma once
#include <vector>
#include <optional>

#include "window.h"

namespace SnowEngine {
	//Contains the surface capabilities, formats and present modes supported by a specific device
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR		capabilities; //informazioni riguardo il numero minimo/massimo della swapchain e risoluzione
		std::vector<VkSurfaceFormatKHR> formats; //formato pixel, colore, ecc.
		std::vector<VkPresentModeKHR>	presentModes; //modalità presentazione immagini
	};
	
	class Device {
    public:
        struct QueueFamilyIndices {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            inline bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
		};
		
	public:
		Device(Window& window);
		~Device();

		//Returns the VkDevice
		inline operator VkDevice() { return GetDevice(); }

		inline static Device& Get() { return *currentDevice; }
		inline VkInstance					GetInstance()			{ return instance; }
		inline VkPhysicalDevice				GetPhysicalDevice()		{ return physicalDevice; }
		//Returns the area of memory where commadBuffers are stored
		inline VkCommandPool				GetCommandPool()		{ return commandPool; }
		//
		inline VkDescriptorPool				GetDescriptorPool()		{ return descriptorPool; }
		inline VkDescriptorPool				GetImGuiDescriptorPool(){ return imguiPool; }
		//Returns the VkDevice
		inline VkDevice						GetDevice()				{ return device; }
		//
		inline VkPhysicalDeviceProperties	GetDeviceProperties()	{ VkPhysicalDeviceProperties properties{}; vkGetPhysicalDeviceProperties(physicalDevice, &properties); return properties; }
		//Gets the value of a specific queue
		inline VkQueue						GetGraphicsQueue()		{ return graphicsQueue; }
		//Gets the value of a specific queue
		inline VkQueue						GetPresentQueue()		{ return presentQueue; }
		//Returns the window surface use by the application
		inline VkSurfaceKHR					GetSurface()			{ return surface; }
		//Returns a set of std::optional values for each queue family in it
		inline QueueFamilyIndices			GetQueueFamilies()		{ return FindQueueFamilies(physicalDevice); }
		//Gets all the available swapChain formats supported by the device
		inline SwapChainSupportDetails		QuerySwapChainSupport() { return QuerySwapChainSupport(physicalDevice); };	
		
		//
		VkCommandBuffer BeginSingleTimeCommands();
		//
		void			CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		//Given its details, creates a buffer and its memory
		void			CreateBuffer(VkDeviceSize size, VkBufferUsageFlags bufferType, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		//
		void			CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	    //
		VkImageView		CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		//
		void			EndSingleTimeCommands(VkCommandBuffer commandBuffer);
        //
		VkFormat		FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	private:
		//
		bool						CheckDeviceExtensionsSupport(VkPhysicalDevice device);
		//Checks the availability of every requested validation layer
		bool						CheckValidationLayerSupport();
		//
		void						CreateCommandPool();
		//
		void						CreateDescriptorPool();
		//
		void						CreateLogicalDevice();
		//
		void						CreateSurface();
		//
		void						FillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		//
		uint32_t					FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		//
		QueueFamilyIndices			FindQueueFamilies(VkPhysicalDevice device);
		//Gets all the available extensions
		void						GetAvailableExtensions();
		//Returns all the glfw extensions plus the validation ones if they are enabled
		std::vector<const char*>	GetRequiredExtension();
		//
		void						InitInstance();	
		//
		bool						IsDeviceSuitable(VkPhysicalDevice device);
		//
		SwapChainSupportDetails		QuerySwapChainSupport(VkPhysicalDevice device);
		//
		void						SelectPhysicalDevice();
		//Creates the manger for the validation layers
		void						SetupValidationCallbacks();	
		
	private:
		static Device* currentDevice;

		Window& window;
		
		VkCommandPool				commandPool;
		VkDebugUtilsMessengerEXT	debugMessenger; //handle della funzione di callback
		VkDescriptorPool			descriptorPool;
		VkDescriptorPool			imguiPool;
		VkDevice					device;
		VkInstance					instance; //connessione tra vulkan e l`applicazione	
		VkPhysicalDevice			physicalDevice;
		VkQueue						graphicsQueue;
		VkQueue						presentQueue;
		VkSurfaceKHR				surface;

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
	};
}