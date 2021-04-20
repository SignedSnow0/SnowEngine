#pragma once
#include <glm/glm.hpp>
#include "Buffers/uniformBuffer.hpp"
#include "model.h"

namespace SnowEngine {
	class Light {
		struct LightUbo {
			glm::vec3 color;
			float ambientStrenght;
			glm::vec3 pos;
			glm::vec3 cameraPos;
			float specularStrength;
		};
	public:
		Light(Device& device);

		void SetCameraPos(glm::vec3 pos) { cameraPos = pos; }

		void SetPos(glm::vec3 pos) { this->pos = pos; }
		inline glm::vec3 GetPos() { return pos; }

		void SetColor(glm::vec3 color);
		inline glm::vec3 GetColor() { return color; }

		void SetAmbientStrength(float strenght);
		inline float GetAmbientStrength() { return ambientStrength; }

		void SetSpecularStrength(float strenght) { specularStrength = strenght; }
		inline float GetSpecularStrength() { return specularStrength; }

		void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, size_t imageIndex);
		void Update(uint32_t frame); 
		inline VkDescriptorSetLayoutBinding GetLayoutBinding() { return uBuffer.GetLayoutBinding(); }
		inline VkWriteDescriptorSet GetDescriptorWrite(uint32_t i , VkDescriptorSet dstSet) { return uBuffer.CreateDescriptorWrite(i, dstSet); }
		inline VkDescriptorSet GetModelDescriptorSet(uint32_t frameIndex) { return model.GetDescriptorSet(frameIndex); }

	private:

	private:
		Device& device;

		LightUbo ubo;
		UniformBuffer<LightUbo> uBuffer{ device, VK_SHADER_STAGE_FRAGMENT_BIT, 1, ubo, 3 };

		glm::vec3 color;
		float ambientStrength = 0.5f;
		glm::vec3 pos = glm::vec3(2.0f);
		glm::vec3 cameraPos;
		float specularStrength = 0.5f;

		Model model{ device, "C:\\dev\\SnowEngine\\Engine\\resources\\models\\sphere.obj" };
	};
}