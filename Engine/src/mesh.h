#pragma once
#include "Buffers/buffer.hpp"
#include "Buffers/texture.h"

namespace SnowEngine {
	class Mesh {
	public:
		Mesh(Device& device, const std::vector<Vertex> vertices, const std::vector<uint16_t> indices, std::vector<Texture*> textures);
		~Mesh();

		inline VkDescriptorSet						GetDescriptorSet(uint32_t frame)			{ return descriptorSets[frame]; }
		inline uint32_t								GetNumIndices()								{ return indexBuffer.GetIndexCount(); }
		std::vector<VkWriteDescriptorSet>			GetDescriptorWrites(VkDescriptorSet dstSet);
		std::vector<VkDescriptorSetLayoutBinding>	GetLayoutBindings();

		void CreateDescriptorSet(VkDescriptorSetLayout layout);
		void Draw(VkCommandBuffer commandBuffer);
	
	private:

	private:
		Device& device;

		VertexBuffer					vertexBuffer;
		IndexBuffer						indexBuffer;
		std::vector<Texture*>			textures;
		std::vector<VkDescriptorSet>	descriptorSets;
	};
}