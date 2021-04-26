#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "model.h"
#include "pipeline.h"
#include "light.h"

namespace SnowEngine {
	class Camera {
		struct Ubo {
			glm::mat4 view;
			glm::mat4 proj;
		};
	public:
		Camera(Device& device, glm::vec3 pos);
		~Camera();

		inline void SetWindowSize(std::pair<float, float> size) { windowSize = size; }
		inline glm::vec3 GetPos() { return pos; }
		//Adds the model to the render queue, after a frame is rendered the queue is emptied
		void BindModel(Model* model);
		void Draw(VkCommandBuffer commandBuffer, size_t frameIndex, VkDescriptorSet globalDescriptors);
		bool Update(uint32_t frame, float deltaTime);
		inline VkDescriptorSetLayoutBinding GetLayoutBinding() { return mvpBuffer.GetLayoutBinding(); }
		inline VkWriteDescriptorSet GetDescriptorWrite(uint32_t i, VkDescriptorSet dstSet) { return mvpBuffer.CreateDescriptorWrite(i, dstSet); }

	private:

	private:
		Device& device;
		
		std::vector<Model*> models;

		Ubo mvpMatrix{};
		UniformBuffer<Ubo> mvpBuffer{ device, VK_SHADER_STAGE_VERTEX_BIT, 0, mvpMatrix, 3 };

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