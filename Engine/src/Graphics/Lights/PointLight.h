#pragma once
#include <glm/glm.hpp>

#include "Vulkan/Buffers/StorageBuffer.hpp"

namespace SnowEngine {
	class PointLight {
	public:
		struct PointLightUBO {
			alignas(4) float Constant = 1.0f;
			alignas(4) float Linear = 0.09f;
			alignas(4) float Quadratic = 0.032;
			alignas(16) glm::vec3 Position = glm::vec3(1.0f);
			alignas(16) glm::vec3 Ambient = glm::vec3(0.05f);
			alignas(16) glm::vec3 Diffuse = glm::vec3(0.8f);
			alignas(16) glm::vec3 Specular = glm::vec3(1.0f);
		};
	public:
		PointLight(Device& device);
		~PointLight() = default;

		inline void SetConstant(size_t index, const float& constant)		{ ubo[index].Constant = constant; }
		inline void SetLinear(size_t index, const float& linear)			{ ubo[index].Linear = linear; }
		inline void SetQuadratic(size_t index, const float& quadratic)		{ ubo[index].Quadratic = quadratic; }
		inline void SetPosition(size_t index, const glm::vec3& position)	{ ubo[index].Position = position; }
		inline void SetAmbient(size_t index, const glm::vec3& ambient)		{ ubo[index].Ambient = ambient; }
		inline void SetDiffuse(size_t index, const glm::vec3& diffuse)		{ ubo[index].Diffuse = diffuse; }
		inline void SetSpecular(size_t index, const glm::vec3& specular)	{ ubo[index].Specular = specular; }
		inline StorageBuffer<PointLightUBO>* GetBuffer()					{ return uBuffer; }
		inline PointLightUBO* GetUBO(size_t index)							{ return &ubo[index]; }
		inline size_t GetBoundLights()										{ return ubo.size(); }

		inline void AddLight(PointLightUBO& data)	{ ubo.push_back(data); }
		inline void RemoveLight(size_t index)		{ ubo.erase(ubo.begin() + index); }
		inline void Update(uint32_t frame)			{ uBuffer->Update(frame, ubo); }
	private:

	private:
		Device& device;

		std::vector<PointLightUBO> ubo{ PointLightUBO{} };
		static StorageBuffer<PointLightUBO>* uBuffer;
	};
}