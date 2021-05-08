#pragma once
#include "Vulkan/Buffers/Buffer.hpp"
#include "Vulkan/Buffers/Texture.h"
#include "Vulkan/Buffers/UniformBuffer.hpp"

namespace SnowEngine {
	class Mesh {
	public:
		struct ProcessingFlags {
			bool HasTransparency = false;
			
			ProcessingFlags() {}
		};
	public:
		Mesh(Device& device, const std::vector<Vertex> vertices, const std::vector<uint16_t> indices, std::vector<Texture*> textures, ProcessingFlags flags = ProcessingFlags());
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
		ProcessingFlags flags;
		UniformBuffer<ProcessingFlags>  flagsBuffer{ device, VK_SHADER_STAGE_FRAGMENT_BIT, 0, flags, 3 };

		VertexBuffer					vertexBuffer;
		IndexBuffer						indexBuffer;
		std::vector<Texture*>			textures;
		std::vector<VkDescriptorSet>	descriptorSets;
	};
}