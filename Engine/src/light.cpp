#include "light.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imguiLib/imgui.h>

namespace SnowEngine {
	Light::Light(Device& device) : device(device) {
		ambientStrength = 1.0f;
		color = glm::vec3(1.0f);
	}

	void Light::SetColor(glm::vec3 color) {
		this->color = color;
	}

	void Light::SetAmbientStrength(float strenght) {
		ambientStrength = strenght;
	}
	void Light::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, size_t imageIndex) {
		auto push = model.GetPushConstant();
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &push);
		model.Draw(commandBuffer, pipelineLayout, imageIndex);
	}

	void Light::Update(uint32_t frame) {
		if (ImGui::Begin("Light")) {
			glm::vec3 color = { GetColor() };
			float col[3] = { color.r, color.g, color.b };
			ImGui::ColorPicker3("Color", col);
			color.r = col[0];
			color.g = col[1];
			color.b = col[2];
			SetColor(color);

			glm::vec3 position = { GetPos() };
			float pos[3] = { position.x, position.y, position.z };
			ImGui::SliderFloat3("Position", pos, -10.0f, 10.0f);
			position.x = pos[0];
			position.y = pos[1];
			position.z = pos[2];
			SetPos(position);

			SetCameraPos(this->pos);

			float spec = GetSpecularStrength();
			ImGui::SliderFloat("Specular strength", &spec, 0.0f, 1.0f);
			SetSpecularStrength(spec);

			float ambient = GetAmbientStrength();
			ImGui::SliderFloat("Ambient strength", &ambient, 0.0f, 1.0f);
			SetAmbientStrength(ambient);

			ImGui::End();
		}

		glm::vec3 rightPos = glm::vec3(-pos.x, pos.y, -pos.z);
		uBuffer.Update(frame, { color, ambientStrength, rightPos, cameraPos, specularStrength });

		glm::mat4 push = glm::translate(glm::mat4(1.0f), pos);
		push *= glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		model.SetPushConstant(push);
	}
}