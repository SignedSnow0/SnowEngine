#include "camera.h"

#include "application.h"
#include "input/keyboard.h"
#include "input/mouse.h"

namespace SnowEngine {
	Camera::Camera(Device& device, glm::vec3 pos, Pipeline::PipelineConfig& pipeline, Light* light) : device(device), pos(pos), light(light) {
		CreateDescriptorSet();

		pipeline.layouts.insert({ 0, descriptorLayout });
		CreatePipeline(pipeline);

		Application& app = Application::Get();
		app.OnUpdate += std::bind(&Camera::Update, this, std::placeholders::_1, std::placeholders::_2);
	}

	Camera::~Camera() {
		vkDestroyDescriptorSetLayout(device, descriptorLayout, nullptr);

		delete pipeline;
	}

	void Camera::BindModel(Model* model) {
		models.push_back(model);
	}

	void Camera::BindLight(Light* light) {
		//lights.push_back(light);
	}

	void Camera::Draw(VkCommandBuffer commandBuffer, size_t frameIndex) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());

		std::vector<VkDescriptorSet> sets = { descriptorSets[frameIndex] };
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), 0, sets.size(), sets.data(), 0, nullptr);

		for (auto model : models) {
			sets = { model->GetDescriptorSet(frameIndex) };
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), 1, sets.size(), sets.data(), 0, nullptr);

			glm::mat4 pushConstant = model->GetPushConstant();
			vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &pushConstant);

			model->Draw(commandBuffer, pipeline->GetLayout(), frameIndex);
		}

		light->Draw(commandBuffer, pipeline->GetLayout(), frameIndex);

		models.clear();
	}

	bool Camera::Update(uint32_t frame, float deltaTime) {
		std::pair<float, float> mouse = Mouse::GetPosition();
		if (mouse.first == -1 || mouse.second == -1) {
			return false;
		}

		float speed = cameraSpeed * deltaTime;

		if (Keyboard::IsKeyPressed(SNOW_KEY_W))
			pos += speed * front;
		if (Keyboard::IsKeyPressed(SNOW_KEY_S))
			pos -= speed * front;
		if (Keyboard::IsKeyPressed(SNOW_KEY_A))
			pos -= glm::normalize(glm::cross(front, up)) * speed;
		if (Keyboard::IsKeyPressed(SNOW_KEY_D))
			pos += glm::normalize(glm::cross(front, up)) * speed;
		if (Keyboard::IsKeyPressed(SNOW_KEY_SPACE))
			pos += speed * up;
		if (Keyboard::IsKeyPressed(SNOW_KEY_LEFT_SHIFT))
			pos -= speed * up;
		
		float xoffset = mouse.first - lastX;
		float yoffset = lastY - mouse.second;
		lastX = mouse.first;
		lastY = mouse.second;
		xoffset *= sensitivity;
		yoffset *= sensitivity;
		yaw += xoffset;
		pitch += yoffset;
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(direction);

		mvpMatrix.view = glm::lookAt(pos, pos + front, up);
		mvpMatrix.proj = glm::perspective(glm::radians(45.0f), windowSize.first / windowSize.second, 0.1f, 1000.0f);
		mvpMatrix.proj[1][1] *= -1;
		mvpBuffer.Update(frame, mvpMatrix);

		return true; //The events keeps on dispatching
	}

	void Camera::CreatePipeline(Pipeline::PipelineConfig& config) {
		pipeline = new Pipeline(device, config);
	}
	void Camera::CreateDescriptorSet() {
		std::vector<VkDescriptorSetLayoutBinding> bindings = { mvpBuffer.GetLayoutBinding(), light->GetLayoutBinding() };

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		createInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorLayout))
			throw std::runtime_error("Failed to create descriptor set layout!");

		std::vector<VkDescriptorSetLayout> layouts(3, descriptorLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = device.GetDescriptorPool();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(3);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(3);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate descriptor sets!");

		for (size_t i = 0; i < 3; i++) {
			std::vector<VkWriteDescriptorSet> descriptorWrites;
			descriptorWrites.push_back(mvpBuffer.CreateDescriptorWrite(i, descriptorSets[i]));
			descriptorWrites.push_back(light->GetDescriptorWrite(i, descriptorSets[i]));

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}	
	}
}