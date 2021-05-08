#include "SwapChain.h"
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <array>

namespace SnowEngine {
    SwapChain::SwapChain(Device& device, VkExtent2D windowExtent) : device(device), windowExtent(windowExtent) {
        CreateSwapChain();
        CreateImageViews();
        CreateDepthResources();
        CreateRenderPass();
        CreateFrameBuffers();
        CreateSyncObjects();
    }
    
    SwapChain::~SwapChain() {
        vkDestroyImage(device, depthImage, nullptr);
        vkDestroyImageView(device, depthImageView, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
        
        for (auto framebuffer : frameBuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);

        vkDestroyRenderPass(device, renderPass, nullptr);

        for (auto imageView : imageViews)
            vkDestroyImageView(device, imageView, nullptr);
        
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    VkResult SwapChain::AcquireImage(uint32_t* imageIndex) {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
        
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, imageIndex);
        
        return result;
    }

    VkResult SwapChain::SubmitCommandBuffer(const VkCommandBuffer* commandBuffer, const uint32_t* imageIndex) {
        if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
            vkWaitForFences(device, 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
            
        imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] }; //i semafori da aspettare prima di disegnare
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] }; //i semafori da attivare una volta che il rendering è finito
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        
        VkSubmitInfo submitInfo{}; //invio al rendering (graphicsQueue)
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = waitStages;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = commandBuffer;

        vkResetFences(device.GetDevice(), 1, &inFlightFences[currentFrame]);
        if (vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit draw command buffer!");

        VkSwapchainKHR swapChains[] = { swapChain };
        
        VkPresentInfoKHR presentInfo{}; //invio alla presentazione (presentQueue)
        presentInfo.sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount  = 1;
        presentInfo.pWaitSemaphores     = signalSemaphores;
        presentInfo.swapchainCount      = 1;
        presentInfo.pSwapchains         = swapChains;
        presentInfo.pImageIndices       = imageIndex;
        presentInfo.pResults            = nullptr; // Optional

        VkResult result = vkQueuePresentKHR(device.GetPresentQueue(), &presentInfo);
        
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        
        return result;
    }

    VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;
        else {
            VkExtent2D actualExtent = windowExtent;
            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats)
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        /*for (const auto& availablePresentMode : availablePresentModes)
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;*/

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    void SwapChain::CreateDepthResources() {
        VkFormat depthFormat = FindDepthFormat();

        device.CreateImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = device.CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT); 
    }

    void SwapChain::CreateFrameBuffers() {
        frameBuffers.resize(imageViews.size());

        for (size_t i = 0; i < imageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = { imageViews[i], depthImageView };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = renderPass; //il render pass con cui il framebuffer deve essere compatibile
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments    = attachments.data(); //le imageviews connesse al attachment del renderpass
            framebufferInfo.width           = swapChainExtent.width;
            framebufferInfo.height          = swapChainExtent.height;
            framebufferInfo.layers          = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create framebuffer!");
        }
    }

    void SwapChain::CreateImageViews() {
        imageViews.resize(images.size());

        for (uint32_t i = 0; i < imageViews.size(); i++)
            imageViews[i] = device.CreateImageView(images[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void SwapChain::CreateRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format          = swapChainImageFormat;
        colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR; //cosa fare prima e dopo renderizzare
        colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //stessa cosa con lo stencil buffer
        colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format          = FindDepthFormat();
        depthAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR; //cosa fare prima e dopo renderizzare
        depthAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //stessa cosa con lo stencil buffer
        depthAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment   = 0;
        colorAttachmentRef.layout       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment   = 1;
        depthAttachmentRef.layout       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass       = VK_SUBPASS_EXTERNAL; //indica il subpass implicito sempre presente
        dependency.dstSubpass       = 0;
        dependency.srcStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask    = 0;
        dependency.dstStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment,depthAttachment };

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount  = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments     = attachments.data();
        renderPassInfo.subpassCount     = 1;
        renderPassInfo.pSubpasses       = &subpass;
        renderPassInfo.dependencyCount  = 1;
        renderPassInfo.pDependencies    = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("failed to create render pass!");
    }

    void SwapChain::CreateSwapChain() {
        SwapChainSupportDetails swapChainSupport = device.QuerySwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats); //ottengo i migliori valori tra quelli disponibili
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; //imposto il numero massimo di immagini contemporanee
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
            imageCount = swapChainSupport.capabilities.maxImageCount; //se il valore è troppo alto lo imposto al massimo disponibile

        Device::QueueFamilyIndices indices = device.GetQueueFamilies();
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
        
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface          = device.GetSurface();
        createInfo.minImageCount    = imageCount;
        createInfo.imageFormat      = surfaceFormat.format;
        createInfo.imageColorSpace  = surfaceFormat.colorSpace;
        createInfo.imageExtent      = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode         = VK_SHARING_MODE_CONCURRENT; //le immagini possono essere condivise tra queue families
            createInfo.queueFamilyIndexCount    = 2;
            createInfo.pQueueFamilyIndices      = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE; //le immagini devono essere passate esplicitamente
            createInfo.queueFamilyIndexCount    = 0; // Optional
            createInfo.pQueueFamilyIndices      = nullptr; // Optional
        }
        createInfo.preTransform     = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //blend nel background con altre app di windows
        createInfo.presentMode      = presentMode;
        createInfo.clipped          = VK_TRUE; //elimina dal buffer pixel coperti da altre finestre
        createInfo.oldSwapchain     = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swap chain!");

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr); //una volta creata la swapChain ottengo le immagini in un vettore
        images.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());
        swapChainImageFormat = surfaceFormat.format; //e le sue informazioni
        swapChainExtent = extent;
    }

    void SwapChain::CreateSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(imageViews.size(), VK_NULL_HANDLE);
        
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS || vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS || vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create semaphores!");
    }

    VkFormat SwapChain::FindDepthFormat() {
        return device.FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}