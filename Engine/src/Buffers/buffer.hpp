#pragma once
#include <iostream>
#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "../device.h"

namespace SnowEngine
{
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };
    
    template<typename T>
    class Buffer {
    public:
        Buffer(Device& device, VkBufferUsageFlags bufferType, const std::vector<T>& data) : device(device) {
            VkDeviceSize bufferSize = sizeof(data[0]) * data.size();
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;

            device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
            
            void* tmp;
		    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &tmp); //permette di accedere a una regione di memoria di un oggetto
		    memcpy(tmp, data.data(), (size_t)bufferSize); //copio i miei dati nel puntatore della memoria
            vkUnmapMemory(device, stagingBufferMemory);

            device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferType, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);

            CopyBuffer(stagingBuffer, buffer, bufferSize);
            
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }
        ~Buffer() {
            vkDestroyBuffer(device, buffer, nullptr);
            vkFreeMemory(device, bufferMemory, nullptr);
        }

        virtual void Bind(VkCommandBuffer commandBuffer) = 0;

    protected:
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool           = device.GetCommandPool();
            allocInfo.commandBufferCount    = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);

            VkBufferCopy copyRegion{};
            copyRegion.srcOffset    = 0; // Optional
            copyRegion.dstOffset    = 0; // Optional
            copyRegion.size         = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(device.GetGraphicsQueue());

            vkFreeCommandBuffers(device, device.GetCommandPool(), 1, &commandBuffer);
        }

    protected:
        Device& device;
        
        VkBuffer buffer;
        VkMemoryRequirements memRequirements;
        VkDeviceMemory bufferMemory;
    };

    class VertexBuffer : Buffer<Vertex> {
    public:
        VertexBuffer(Device& device, const std::vector<Vertex>& vertices) : Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices), vertexCount(vertices.size()) { }
        ~VertexBuffer() { }

        inline uint32_t GetVertexCount() { return vertexCount; }

        //Binds the current buffer (to use in commandbuffer recording)
        virtual void Bind(VkCommandBuffer commandBuffer) override {
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, offsets);
        }
        
        static VkVertexInputBindingDescription GetBindingDescription() {
            VkVertexInputBindingDescription description{};
            description.binding     = 0;
            description.stride      = sizeof(Vertex);
            description.inputRate   = VK_VERTEX_INPUT_RATE_VERTEX;
            return description;
        }
        
        static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};           
            attributeDescriptions[0].binding    = 0;
            attributeDescriptions[0].location   = 0;
            attributeDescriptions[0].format     = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset     = offsetof(Vertex, pos);

            attributeDescriptions[1].binding    = 0;
            attributeDescriptions[1].location   = 1;
            attributeDescriptions[1].format     = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset     = offsetof(Vertex, normal);

            attributeDescriptions[2].binding    = 0;
            attributeDescriptions[2].location   = 2;
            attributeDescriptions[2].format     = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset     = offsetof(Vertex, texCoord);

            return attributeDescriptions;
        }

    private:

    private:
        uint32_t vertexCount;
    };

    class IndexBuffer : Buffer<uint16_t> {
    public:
        IndexBuffer(Device& device, const std::vector<uint16_t>& indices) : Buffer(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices), indexCount(indices.size()) { }
        ~IndexBuffer() { }

        inline uint32_t GetIndexCount() { return indexCount; }

        //Binds the current buffer (to use in commandbuffer recording)
        virtual void Bind(VkCommandBuffer commandBuffer) override {
            vkCmdBindIndexBuffer(commandBuffer, buffer, 0, VK_INDEX_TYPE_UINT16);
        }
        
        static VkVertexInputBindingDescription GetBindingDescription() {
            VkVertexInputBindingDescription description{};
            description.binding     = 0;
            description.stride      = sizeof(Vertex);
            description.inputRate   = VK_VERTEX_INPUT_RATE_VERTEX;
            return description;
        }
        
        static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};           
            attributeDescriptions[0].binding    = 0;
            attributeDescriptions[0].location   = 0;
            attributeDescriptions[0].format     = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset     = offsetof(Vertex, pos);

            attributeDescriptions[1].binding    = 0;
            attributeDescriptions[1].location   = 1;
            attributeDescriptions[1].format     = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset     = offsetof(Vertex, normal);
            return attributeDescriptions;
        }

    private:

    private:
        uint32_t indexCount;
    };
}
