#include "light.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imguiLib/imgui.h>

namespace SnowEngine {
	Light::Light(Device& device) : device(device) {

	}

	Light::~Light() {

	}

	void Light::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, size_t imageIndex) {
		auto push = model.GetPushConstant();
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &push);
		//model.Draw(commandBuffer, imageIndex);
	}

	void Light::Update(uint32_t frame, glm::vec3 camPos) {
		SetCameraPos(camPos);

		if (ImGui::Begin("Ambient Light")) {
			glm::vec3 color = { GetColor() };
			float col[3] = { color.r, color.g, color.b };
			ImGui::ColorPicker3("Color", col, ImGuiColorEditFlags_PickerHueWheel);
			color.r = col[0];
			color.g = col[1];
			color.b = col[2];
			SetColor(color);

			glm::vec3 position = { GetPos() };
			float pos[3] = { position.x, position.y, position.z };
			ImGui::SliderFloat3("Position", pos, -50.0f, 50.0f);
			position.x = pos[0];
			position.y = pos[1];
			position.z = pos[2];
			SetPos(position);
			model.SetTranslation(position);

			float spec = GetSpecularStrength();
			ImGui::SliderFloat("Specular strength", &spec, 0.0f, 1.0f);
			SetSpecularStrength(spec);

			float ambient = GetAmbientStrength();
			ImGui::SliderFloat("Ambient strength", &ambient, 0.0f, 1.0f);
			SetAmbientStrength(ambient);		
		}
		ImGui::End();
		uBuffer.Update(frame, { color, ambientStrength, pos, cameraPos, specularStrength });

		model.SetScale(glm::vec3(0.5f));
		model.Update();
	}
}