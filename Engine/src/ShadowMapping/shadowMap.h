#pragma once
#include <glm/glm.hpp>

#include "device.h"
#include "pipeline.h"
#include "model.h"
#include "Buffers/uniformBuffer.hpp"

#define SHADOW_MAP_WIDTH 2048
#define SHADOW_MAP_HEIGHT 2048

namespace SnowEngine {
	class ShadowMap {
	public:
		struct InUBO {
			glm::mat4 LightVP = glm::mat4(1.0f);

			InUBO() = default;
			InUBO(const glm::mat4& lightVP) : LightVP(lightVP) {}
		};
	public:
		ShadowMap(Device& device);

		void RenderShadowMap(uint32_t frame, const VkCommandBuffer* buffer, glm::mat4 LightVP, std::vector<Model*>& models);
	private:
		void CreateImageResources();
		void CreateRenderPass();
		void CreateFramebuffer();
		void CreateUniforms();
		void CreatePipeline();

	private:
		Device& device;
		Pipeline* shadowPipeline;

		VkRenderPass renderPass;
		VkFramebuffer framebuffer;
		VkImage shadowImage;
		VkImageView shadowImageView;
		VkDeviceMemory shadowMemory;
		VkSampler shadowSampler;

		InUBO inUbo;
		UniformBuffer<InUBO> uBufferIn{ device, VK_SHADER_STAGE_VERTEX_BIT, 0, inUbo, 3 };
		std::vector<VkDescriptorSet> inSets;
		VkDescriptorSetLayout inLayout;
	};
}