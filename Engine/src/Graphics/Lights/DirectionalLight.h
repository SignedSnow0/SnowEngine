#pragma once
#include <glm/glm.hpp>

#include <Vulkan/Buffers/UniformBuffer.hpp>

namespace SnowEngine {
	class DirectionalLight {
	public:
		struct DirectionalLightUBO {
			alignas(16) glm::vec3 Direction = glm::vec3(-0.2f);
			alignas(16) glm::vec3 Ambient = glm::vec3(0.05f);
			alignas(16) glm::vec3 Diffuse = glm::vec3(0.4f);
			alignas(16) glm::vec3 Specular = glm::vec3(0.5f);
		};
	public:
		DirectionalLight(Device& device);
		~DirectionalLight() = default;

		inline void SetDirection(const glm::vec3& direction)	{ ubo.Direction = direction; }
		inline glm::vec3 GetDirection()							{ return ubo.Direction; }
		inline void SetAmbient(const glm::vec3& ambient)		{ ubo.Ambient = ambient; }
		inline void SetDiffuse(const glm::vec3& diffuse)		{ ubo.Diffuse = diffuse; }
		inline void SetSpecular(const glm::vec3& specular)		{ ubo.Specular = specular; }
		inline UniformBuffer<DirectionalLightUBO>* GetBuffer() { return &uBuffer; }
		inline DirectionalLightUBO* GetUBO() { return &ubo; }

		inline void Update(uint32_t frame) { uBuffer.Update(frame, ubo); }
	private:		

	private:
		Device& device;

		DirectionalLightUBO ubo{};
		UniformBuffer<DirectionalLightUBO> uBuffer{ device, VK_SHADER_STAGE_FRAGMENT_BIT, 2, ubo, 3 };
	};
}