#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "model.h"
#include "pipeline.h"

namespace SnowEngine {
	class Camera {
		struct VertUBO {
			glm::mat4 view;
			glm::mat4 proj;
		};
		struct FragUBO {
			alignas(16) glm::vec3 position;
		};
	public:
		Camera(Device& device, glm::vec3 pos);
		~Camera();

		inline void SetWindowSize(std::pair<float, float> size) { windowSize = size; }
		inline glm::vec3 GetPos() { return pos; }
		inline glm::mat4 GetViewProj() { return viewPosMatrix.proj * viewPosMatrix.view; }
		//Adds the model to the render queue, after a frame is rendered the queue is emptied
		void BindModel(Model* model);
		void Draw(VkCommandBuffer commandBuffer, size_t frameIndex, VkDescriptorSet globalDescriptors);
		bool Update(uint32_t frame, float deltaTime);
		inline std::vector<VkDescriptorSetLayoutBinding> GetLayoutBindings() { return{ uViewPos.GetLayoutBinding(), uPosition.GetLayoutBinding() }; }
		inline std::vector<VkWriteDescriptorSet> GetDescriptorWrites(uint32_t i, VkDescriptorSet dstSet) { return { uViewPos.CreateDescriptorWrite(i, dstSet), uPosition.CreateDescriptorWrite(i, dstSet) }; }

	private:

	private:
		Device& device;
		
		std::vector<Model*> models;

		VertUBO viewPosMatrix{};
		UniformBuffer<VertUBO> uViewPos{ device, VK_SHADER_STAGE_VERTEX_BIT, 0, viewPosMatrix, 3 };
		FragUBO position{};
		UniformBuffer<FragUBO> uPosition{ device, VK_SHADER_STAGE_FRAGMENT_BIT, 1, position, 3 };

		std::pair<float, float> windowSize{ 1920, 1080 };
		float lastX = 400, lastY = 300;
		float yaw = -90.0f; //asse y
		float pitch = 0.0f; //asse x
		glm::vec3 pos;
		const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 front;
		const float cameraSpeed = 20.5f;
		const float sensitivity = 0.1f;
	};
}