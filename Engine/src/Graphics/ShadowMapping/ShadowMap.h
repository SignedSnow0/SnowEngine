#pragma once
#include <glm/glm.hpp>

#include "Vulkan/Device.h"
#include "Vulkan/Pipeline.h"
#include "Graphics/Model.h"
#include "Vulkan/Buffers/UniformBuffer.hpp"

#define SHADOW_MAP_WIDTH 4096
#define SHADOW_MAP_HEIGHT 4096

#define SHADOW_MAP_CASCADE_COUNT 4

namespace SnowEngine {
	class ShadowMap {
	public:
		struct InUBO {
			glm::mat4 LightVP = glm::mat4(1.0f);

			InUBO() = default;
			InUBO(const glm::mat4& lightVP) : LightVP(lightVP) {}
		};
	public:
		ShadowMap(Device& device, uint32_t matrixbinding, uint32_t imageBinding, VkShaderStageFlags matrixStage, VkShaderStageFlags imageStage);

		inline void SetConstantBias(const float& value) { bias = value; }
		inline void SetBiasSlope(const float& value) { slopeBias = value; }
		inline float GetConstantBias() { return bias; }
		inline float GetBiasSlope() { return slopeBias; }
		inline VkPipelineLayout GetLayout() { return shadowPipeline->GetLayout(); }
		inline std::vector<VkDescriptorSetLayoutBinding> GetLayoutBindings() { return{ imageBinding, uBufferOut->GetLayoutBinding() }; }
		std::vector<VkWriteDescriptorSet> GetDescriptorWrites(uint32_t frame, VkDescriptorSet dstSet);
		void BeginRenderPass(uint32_t frame, VkCommandBuffer buffer, glm::mat4 LightVP);
		void EndRenderPass(uint32_t frame, VkCommandBuffer buffer);
		void RenderShadowMap(VkCommandBuffer buffer, Model* model);
	private:
		void CreateImageResources();
		void CreateRenderPass();
		void CreateFramebuffer();
		void CreateUniforms();
		void CreatePipeline();
		void CreateBindings(uint32_t binding, VkShaderStageFlags imageStage);

	private:
		Device& device;
		Pipeline* shadowPipeline;

		float bias = 1.25f;
		float slopeBias = 1.75f;

		VkRenderPass renderPass;
		VkFramebuffer framebuffer;

		uint32_t imageLocation;
		VkDescriptorImageInfo imageInfo{};
		VkDescriptorSetLayoutBinding imageBinding;
		VkImage shadowImage;
		VkImageView shadowImageView;
		VkDeviceMemory shadowMemory;
		VkSampler shadowSampler;

		InUBO inUbo;
		UniformBuffer<InUBO> uBufferIn{ device, VK_SHADER_STAGE_VERTEX_BIT, 0, inUbo, 3 };
		UniformBuffer<InUBO>* uBufferOut;
		std::vector<VkDescriptorSet> inSets;
		VkDescriptorSetLayout inLayout;
	};
}