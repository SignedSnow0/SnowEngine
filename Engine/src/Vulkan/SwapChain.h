#pragma once
#include <vulkan\vulkan.h>

#include "Device.h"

namespace SnowEngine {

    class SwapChain {
    public:
        SwapChain(Device& device, VkExtent2D windowExtent);
        ~SwapChain();

        //Returns the area of the swapchain
        inline VkExtent2D       GetExtent()                 { return windowExtent; }
        //Return the framebuffer
        inline VkFramebuffer    GetFrameBuffer(size_t i)    { return frameBuffers[i]; }
        //Returns the number of images in the swapchain
        inline size_t           GetImageCount()             { return imageViews.size(); }     
        //Returns the group of attachments and subpasses in a framebuffer
        inline VkRenderPass     GetRenderPass()             { return renderPass; }
       

        //Changes the index the the first image ready for rendering
        VkResult AcquireImage(uint32_t* imageIndex);
        //Submits a recorded commandbuffer for rendering and presentation
        VkResult SubmitCommandBuffer(const VkCommandBuffer* commandBuffer, const uint32_t* imageIndex);
        
    private:
        VkExtent2D          ChooseSwapExtent        (const VkSurfaceCapabilitiesKHR& capabilities);
        VkSurfaceFormatKHR  ChooseSwapSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR    ChooseSwapPresentMode   (const std::vector<VkPresentModeKHR>& availablePresentModes);
        void                CreateDepthResources();
        void                CreateFrameBuffers();
        void                CreateImageViews();
        void                CreateRenderPass();
        void                CreateSwapChain();
        void                CreateSyncObjects();   
        VkFormat            FindDepthFormat();
        
    private:
        Device& device;
        
        std::vector<VkImage>        images; //info riguardo le immagini della swapchain
        std::vector<VkImageView>    imageViews;
        std::vector<VkFramebuffer>  frameBuffers;
        VkImage                     depthImage;
        VkDeviceMemory              depthImageMemory;
        VkImageView                 depthImageView;
        
        VkSwapchainKHR  swapChain;     
        VkFormat        swapChainImageFormat;
        VkExtent2D      windowExtent;
        VkExtent2D      swapChainExtent;
        VkRenderPass    renderPass; //contieme l`insieme di attachments necessari per un framebuffer

        std::vector<VkSemaphore>    imageAvailableSemaphores; //indica che l`immagine è stata ottenuta ed è pronta
        std::vector<VkSemaphore>    renderFinishedSemaphores; //indica che l`immagine è pronta per la presentazione
        std::vector<VkFence>        inFlightFences;
        std::vector<VkFence>        imagesInFlight;
        const int                   MAX_FRAMES_IN_FLIGHT    = 2;
        size_t                      currentFrame            = 0;
    };
}