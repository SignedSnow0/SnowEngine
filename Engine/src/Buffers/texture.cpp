#include "texture.h"
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace SnowEngine {
    Texture::Texture(Device& device, VkShaderStageFlags shaderTarget, uint32_t binding, const std::string& path) : device(device), binding(binding), path(path) {
        auto abs = std::filesystem::absolute(path);
        CreateTextureImage(abs.string());
        CreateViewAndSampler();
        CreateBinding(shaderTarget, binding);
    }

    Texture::~Texture() {        
        vkDestroySampler(device, textureSampler, nullptr);
        
        vkDestroyImageView(device, textureImageView, nullptr);
        
        vkDestroyImage(device, textureImage, nullptr);
        vkFreeMemory(device, textureImageMemory, nullptr);
    }

    void Texture::CreateTextureImage(const std::string& path) {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels)
            throw std::runtime_error("Failed to load texture image! at " + path);

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        device.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* tmp; //metto i pixel dell`immagine in un buffer
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &tmp);
        memcpy(tmp, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);

        stbi_image_free(pixels);

        device.CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

        TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }
    
    void Texture::CreateViewAndSampler() {
        textureImageView = device.CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

        auto properties = device.GetDeviceProperties();
        
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = VK_FILTER_LINEAR;
        samplerInfo.minFilter               = VK_FILTER_LINEAR;
        samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable        = VK_TRUE;
        samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias              = 0.0f;
        samplerInfo.minLod                  = 0.0f;
        samplerInfo.maxLod                  = 0.0f;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
            throw std::runtime_error("Failed to create texture sampler!");
    }

    void Texture::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = device.BeginSingleTimeCommands();

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;
    
        VkImageMemoryBarrier barrier{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout                       = oldLayout;
        barrier.newLayout                       = newLayout;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.image                           = image;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;
        
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else
            throw std::invalid_argument("Unsupported layout transition!");

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        
        device.EndSingleTimeCommands(commandBuffer);
    }

    void Texture::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = device.BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset                     = 0;
        region.bufferRowLength                  = 0;
        region.bufferImageHeight                = 0;
        region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel        = 0;
        region.imageSubresource.baseArrayLayer  = 0;
        region.imageSubresource.layerCount      = 1;
        region.imageOffset                      = { 0, 0, 0 };
        region.imageExtent                      = { width, height, 1 };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        
        device.EndSingleTimeCommands(commandBuffer);
    }

    VkWriteDescriptorSet Texture::GetDescriptorWrite(VkDescriptorSet descriptorSet) {
        imageInfo.imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView     = textureImageView;
        imageInfo.sampler       = textureSampler;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet          = descriptorSet;
        descriptorWrite.dstBinding      = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo      = &imageInfo;

        return descriptorWrite;
    }

    void Texture::CreateBinding(VkShaderStageFlags shaderTarget, uint32_t binding) {
        layoutBinding.binding            = binding;
        layoutBinding.descriptorCount    = 1;
        layoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBinding.pImmutableSamplers = nullptr;
        layoutBinding.stageFlags         = shaderTarget;
    }
}
