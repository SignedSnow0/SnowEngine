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
		Camera(Device& device, glm::vec3 pos, Pipeline::PipelineConfig& pipeline, Light* light);
		~Camera();

		inline void SetWindowSize(std::pair<float, float> size) { windowSize = size; }

		//Adds the model to the render queue, after a frame is rendered the queue is emptied
		void BindModel(Model* model);
		void BindLight(Light* light);
		void Draw(VkCommandBuffer commandBuffer, size_t frameIndex);
		bool Update(uint32_t frame, float deltaTime);

	private:
		void CreatePipeline(Pipeline::PipelineConfig& config);
		void CreateDescriptorSet();

	private:
		Device& device;
		Pipeline* pipeline;
		std::vector<VkDescriptorSet> descriptorSets;
		VkDescriptorSetLayout descriptorLayout;
		
		std::vector<Model*> models;
		Light* light;

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