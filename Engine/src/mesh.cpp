#include "mesh.h"

namespace SnowEngine {
	Mesh::Mesh(Device& device, const std::vector<Vertex> vertices, const std::vector<uint16_t> indices, std::vector<Texture*> textures, ProcessingFlags flags)
		: device(device), vertexBuffer(device, vertices), indexBuffer(device, indices), textures(textures), flags(flags) {

	}

	Mesh::~Mesh() {
		vkFreeDescriptorSets(device, device.GetDescriptorPool(), 3, descriptorSets.data());

		for (Texture* texture : textures)
			delete texture;
	}

	void Mesh::Draw(VkCommandBuffer commandBuffer) {
		if (flags.HasTransparency)
			device.vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);

		vertexBuffer.Bind(commandBuffer);
		indexBuffer.Bind(commandBuffer);

		vkCmdDrawIndexed(commandBuffer, indexBuffer.GetIndexCount(), 1, 0, 0, 0);

		if (flags.HasTransparency)
			device.vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_BACK_BIT);
	}

	std::vector<VkWriteDescriptorSet> Mesh::GetDescriptorWrites(VkDescriptorSet dstSet) {
		std::vector<VkWriteDescriptorSet> writes;
		for (auto texture : textures)
			writes.push_back(texture->GetDescriptorWrite(dstSet));
		return writes;
	}

	std::vector<VkDescriptorSetLayoutBinding> Mesh::GetLayoutBindings() {
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		for (auto texture : textures)
			bindings.push_back(texture->GetLayoutBinding());

		bindings.push_back(flagsBuffer.GetLayoutBinding());
		return bindings;
	}

	void Mesh::CreateDescriptorSet(VkDescriptorSetLayout layout) {	
		std::vector<VkDescriptorSetLayout> layouts(3, layout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = device.GetDescriptorPool();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(3);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate descriptor sets!");

		for (size_t i = 0; i < 3; i++) {
			std::vector<VkWriteDescriptorSet> descriptorWrites = { GetDescriptorWrites(descriptorSets[i]) };
			descriptorWrites.push_back(flagsBuffer.CreateDescriptorWrite(i, descriptorSets[i]));

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
}