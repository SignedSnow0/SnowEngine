#pragma once
#include <glm/glm.hpp>

#include "Buffers/uniformBuffer.hpp"

namespace SnowEngine {
	class SpotLight {
	public:
		struct SpotLightUBO {
			alignas(4) float CutOff = glm::cos(glm::radians(12.5f));
			alignas(4) float OuterCutOff = glm::cos(glm::radians(15.0f));
			alignas(4) float Constant = 1.0f;
			alignas(4) float Linear = 0.09f;
			alignas(4) float Quadratic = 0.032f;
			alignas(16) glm::vec3 Position = glm::vec3(0.0f);
			alignas(16) glm::vec3 Direction = glm::vec3(-1.0f);
			alignas(16) glm::vec3 Ambient = glm::vec3(0.0f);
			alignas(16) glm::vec3 Diffuse = glm::vec3(1.0f);
			alignas(16) glm::vec3 Specular = glm::vec3(1.0f);
		};
	public:
		SpotLight(Device& device);
		~SpotLight();

		inline void SetCutOff(const float& cutOff)				{ ubo.CutOff = cutOff; }
		inline void SetOuterCutOff(const float& outerCutOff)	{ ubo.OuterCutOff = outerCutOff; }
		inline void SetConstant(const float& constant)			{ ubo.Constant = constant; }
		inline void SetLinear(const float& linear)				{ ubo.Linear = linear; }
		inline void SetQuadratic(const float& quadratic)		{ ubo.Quadratic = quadratic; }
		inline void SetPosition(const glm::vec3& position)		{ ubo.Position = position; }
		inline void SetAmbient(const glm::vec3& ambient)		{ ubo.Ambient = ambient; }
		inline void SetDiffuse(const glm::vec3& diffuse)		{ ubo.Diffuse = diffuse; }
		inline void SetSpecular(const glm::vec3& specular)		{ ubo.Specular = specular; }
		inline UniformBuffer<SpotLightUBO>* GetBuffer() { return &uBuffer; }
		inline SpotLightUBO* GetUBO() { return &ubo; }

		inline void Update(uint32_t frame) { uBuffer.Update(frame, ubo); }
	private:

	private:
		Device& device;

		SpotLightUBO ubo{};
		UniformBuffer<SpotLightUBO> uBuffer{ device, VK_SHADER_STAGE_FRAGMENT_BIT, 4, ubo, 3 };
	};
}