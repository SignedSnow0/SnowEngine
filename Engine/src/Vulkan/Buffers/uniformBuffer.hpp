#pragma once
#include <vector>

#include <vulkan/vulkan.h>

#include "Buffer.hpp"

namespace SnowEngine {
	template<typename T>
	class UniformBuffer {
	public:
		UniformBuffer(Device& device, VkShaderStageFlags shaderTarget, uint32_t binding, const T& data, size_t swapchainImagesCount);
		~UniformBuffer();

		inline VkDescriptorSetLayoutBinding GetLayoutBinding() { return layoutBinding; }

		void Update(uint32_t imageIndex, const T& data);
		VkWriteDescriptorSet CreateDescriptorWrite(uint32_t i, VkDescriptorSet dstSet);

	private:
		void CreateBinding(VkShaderStageFlags shaderTarget, uint32_t binding);
		void CreateUniformBuffers();

	private:
		Device& device;

		VkDescriptorBufferInfo bufferInfo{};
		VkDescriptorSetLayoutBinding layoutBinding{};

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VmaAllocation> uniformBuffersAllocations;

		size_t swapchainImagesCount;
		uint32_t binding;
		const T& data;
	};
}

/////////////////////////////////////////////////////Implementation////////////////////////////////////////////////////

namespace SnowEngine {
	template<typename T>
	UniformBuffer<T>::UniformBuffer(Device& device, VkShaderStageFlags shaderTarget, uint32_t binding, const T& data, size_t swapchainImagesCount)
		: device(device), swapchainImagesCount(swapchainImagesCount), data(data), binding(binding) {
		CreateUniformBuffers();
		CreateBinding(shaderTarget, binding);
	}

	template<typename T>
	UniformBuffer<T>::~UniformBuffer() {
		for (size_t i = 0; i < swapchainImagesCount; i++) {
			vmaDestroyBuffer(device, uniformBuffers[i], uniformBuffersAllocations[i]);
		}
	}

	template<typename T>
	void UniformBuffer<T>::Update(uint32_t imageIndex, const T& data) {
		void* tmp;
		vmaMapMemory(device, uniformBuffersAllocations[imageIndex], &tmp);
		memcpy(tmp, &data, sizeof(data));
		vmaUnmapMemory(device, uniformBuffersAllocations[imageIndex]);
	}

	template<typename T>
	VkWriteDescriptorSet UniformBuffer<T>::CreateDescriptorWrite(uint32_t i, VkDescriptorSet dstSet) {
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(T);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = dstSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		return descriptorWrite;
	}

	template<typename T>
	void UniformBuffer<T>::CreateBinding(VkShaderStageFlags shaderTarget, uint32_t binding) {
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = shaderTarget;
		layoutBinding.pImmutableSamplers = nullptr;
	}

	template<typename T>
	void UniformBuffer<T>::CreateUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(T);

		uniformBuffers.resize(swapchainImagesCount);
		uniformBuffersAllocations.resize(swapchainImagesCount);

		for (size_t i = 0; i < swapchainImagesCount; i++)
			device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffers[i], &uniformBuffersAllocations[i]);
	}
}