#include "mesh.h"

namespace SnowEngine {
	Mesh::Mesh(Device& device, const std::vector<Vertex> vertices, const std::vector<uint16_t> indices, std::vector<Texture*> textures) : device(device), vertexBuffer(device, vertices), indexBuffer(device, indices), textures(textures) {

	}

	Mesh::~Mesh() {
		//for (Texture* texture : textures)
		//	delete texture;
	}

	void Mesh::Draw(VkCommandBuffer commandBuffer) {
		vertexBuffer.Bind(commandBuffer);
		indexBuffer.Bind(commandBuffer);

		vkCmdDrawIndexed(commandBuffer, indexBuffer.GetIndexCount(), 1, 0, 0, 0);
	}

	std::vector<VkWriteDescriptorSet> Mesh::GetDescriptorWrites(VkDescriptorSet dstSet) {
		std::vector<VkWriteDescriptorSet> writes;
		/*if (textures.size() > 0)
			writes.push_back(textures[0]->GetDescriptorWrite(dstSet));*/
		for (auto texture : textures)
			writes.push_back(texture->GetDescriptorWrite(dstSet));
		return writes;
	}

	std::vector<VkDescriptorSetLayoutBinding> Mesh::GetLayoutBindings()
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
	/*	if(textures.size() > 0)
			bindings.push_back(textures[0]->GetLayoutBinding());*/
		for (auto texture : textures)
			bindings.push_back(texture->GetLayoutBinding());
		return bindings;
	}
}