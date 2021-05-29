#pragma once
#include <glm/glm.hpp>

#include "Vulkan/Device.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Buffers/Buffer.hpp"
#include "Vulkan/Buffers/UniformBuffer.hpp"

namespace SnowEngine {
	class Skybox {
	public:
		struct SkyboxUBO {
			glm::mat4 CameraView = glm::mat4(1.0f);
			glm::mat4 CameraProj = glm::mat4(1.0f);

			SkyboxUBO() = default;
			SkyboxUBO(const glm::mat4& cameraView, const glm::mat4& cameraProj) : CameraView(cameraView), CameraProj(cameraProj) {}
		};
	public:
		Skybox(Device& device, std::vector<std::string> textures, uint32_t binding, VkShaderStageFlags shaderTarget);
		~Skybox();

		std::vector<VkWriteDescriptorSet> GetDescriptorWrite(uint32_t i, VkDescriptorSet descriptorSet);

		void Draw(uint32_t frame, VkCommandBuffer buffer, const glm::mat4& view, const glm::mat4& proj);
	private:
		void CreateImage(const std::vector<std::string>& textures);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void CreateImageView();
		void CreateSampler();
		void CreateBinding(VkShaderStageFlags shaderTarget);
		void CreatePipeline();

	private:
		class SkyVertexBuffer : Buffer<float> {
		public:
			SkyVertexBuffer(Device& device, const std::vector<float>& vertices) : Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices), vertexCount(vertices.size()) { }

			inline virtual void Bind(VkCommandBuffer commandBuffer) override {
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, offsets);
			}

			static VkVertexInputBindingDescription GetBindingDescription();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

		private:
			uint32_t vertexCount;
		};
		SkyVertexBuffer* vBuffer;

		Device& device;
		Pipeline* pipeline;

		SkyboxUBO ubo{};
		UniformBuffer<SkyboxUBO> uBuffer{ device, VK_SHADER_STAGE_VERTEX_BIT, 0, ubo, 3 };

		VkImage image;
		VmaAllocation allocation;
		VkImageView imageView;
		VkSampler imageSampler;

		VkDescriptorImageInfo imageInfo{};
		VkDescriptorSetLayoutBinding layoutBinding{};
		VkDescriptorSetLayout layout;
		std::vector<VkDescriptorSet> descriptorSets;
		uint32_t binding;
	};
}