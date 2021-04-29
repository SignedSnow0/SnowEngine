#include "camera.h"

#include "application.h"
#include "input/keyboard.h"
#include "input/mouse.h"

namespace SnowEngine {
	Camera::Camera(Device& device, glm::vec3 pos) : device(device), pos(pos) {
		Application& app = Application::Get();
		app.OnUpdate += std::bind(&Camera::Update, this, std::placeholders::_1, std::placeholders::_2);
	}

	Camera::~Camera() {

	}

	void Camera::BindModel(Model* model) {
		models.push_back(model);
	}

	void Camera::Draw(VkCommandBuffer commandBuffer, size_t frameIndex, VkDescriptorSet globalDescriptor) {
		for (auto model : models) {
			//assert(model->GetPipeline() != nullptr && "Model must have a pipeline bound before drawing!");

			//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, model->GetPipeline()->GetPipeline());

			//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, model->GetPipeline()->GetLayout(), 0, 1, &globalDescriptor, 0, nullptr);

			//model->Draw(commandBuffer, frameIndex);
		}

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

		viewPosMatrix.view = glm::lookAt(pos, pos + front, up);
		viewPosMatrix.proj = glm::perspective(glm::radians(45.0f), windowSize.first / windowSize.second, 0.1f, 1000.0f);
		viewPosMatrix.proj[1][1] *= -1;

		uViewPos.Update(frame, viewPosMatrix);
		uPosition.Update(frame, { pos });
		return true; //The events keeps on dispatching
	}
}