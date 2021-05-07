#pragma once
#include <string>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include "device.h"

namespace SnowEngine {
    class Texture {
    public:
        Texture(Device& device, VkShaderStageFlags shaderTarget, uint32_t binding, const std::string& path);
        ~Texture();

        inline VkDescriptorSetLayoutBinding GetLayoutBinding() { return layoutBinding; }
        inline std::string GetPath() { return path; }
        VkWriteDescriptorSet GetDescriptorWrite(VkDescriptorSet descriptorSet);     
        
    private:
        void CreateTextureImage(const std::string& path);
        void CreateViewAndSampler();
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void CreateBinding(VkShaderStageFlags shaderTarget, uint32_t binding);
        
    private:
        Device& device;

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        VkDescriptorImageInfo imageInfo{};
        
        VkDescriptorSetLayoutBinding layoutBinding{};
        uint32_t binding;

        std::string path;
    };
}