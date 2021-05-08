#pragma once
#include "Vulkan/Device.h"

namespace SnowEngine {
	template<typename T>
	class StorageBuffer {
	public:
		StorageBuffer(Device& device, VkShaderStageFlags shaderTarget, uint32_t binding, size_t swapchainImagesCount);
		~StorageBuffer();

		inline VkDescriptorSetLayoutBinding GetLayoutBinding() { return layoutBinding; }

		void Update(uint32_t imageIndex, std::vector<T>& data);
		VkWriteDescriptorSet CreateDescriptorWrite(uint32_t i, VkDescriptorSet dstSet);

	private:
		void CreateBinding(VkShaderStageFlags shaderTarget, uint32_t binding);
		void CreateStorageBuffers();

	private:
		Device& device;

		VkDescriptorBufferInfo bufferInfo{};
		VkDescriptorSetLayoutBinding layoutBinding{};

		std::vector<VkBuffer> storageBuffers;
		std::vector<VkDeviceMemory> storageBuffersMemory;

		size_t swapchainImagesCount;
		uint32_t binding;
	};
}

/////////////////////////////////////////////////////Implementation////////////////////////////////////////////////////

namespace SnowEngine {
	template<typename T>
	StorageBuffer<T>::StorageBuffer(Device& device, VkShaderStageFlags shaderTarget, uint32_t binding, size_t swapchainImagesCount)
		: device(device), swapchainImagesCount(swapchainImagesCount), binding(binding) {
		CreateStorageBuffers();
		CreateBinding(shaderTarget, binding);
	}

	template<typename T>
	StorageBuffer<T>::~StorageBuffer() {
		for (size_t i = 0; i < swapchainImagesCount; i++) {
			vkDestroyBuffer(device, storageBuffers[i], nullptr);
			vkFreeMemory(device, storageBuffersMemory[i], nullptr);
		}
	}

	template<typename T>
	void StorageBuffer<T>::Update(uint32_t imageIndex, std::vector<T>& data) {
		void* tmp;
		vkMapMemory(device, storageBuffersMemory[imageIndex], 0, sizeof(T) * data.size(), 0, &tmp);
		memcpy(tmp, data.data(), sizeof(T) * data.size());
		vkUnmapMemory(device, storageBuffersMemory[imageIndex]);
	}

	template<typename T>
	VkWriteDescriptorSet StorageBuffer<T>::CreateDescriptorWrite(uint32_t i, VkDescriptorSet dstSet) {
		bufferInfo.buffer = storageBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(T);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = dstSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		return descriptorWrite;
	}

	template<typename T>
	void StorageBuffer<T>::CreateBinding(VkShaderStageFlags shaderTarget, uint32_t binding) {
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = shaderTarget;
		layoutBinding.pImmutableSamplers = nullptr;
	}

	template<typename T>
	void StorageBuffer<T>::CreateStorageBuffers() {
		VkDeviceSize bufferSize = sizeof(T);

		storageBuffers.resize(swapchainImagesCount);
		storageBuffersMemory.resize(swapchainImagesCount);

		for (size_t i = 0; i < swapchainImagesCount; i++)
			device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, storageBuffers[i], storageBuffersMemory[i]);
	}
}