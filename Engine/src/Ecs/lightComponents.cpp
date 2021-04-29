#include "lightComponents.h"
#include "componentUtils.hpp"

namespace SnowEngine {
	void DirectionalLightComponent::ImGuiDraw() {
		DirectionalLight::DirectionalLightUBO* ubo = Light->GetUBO();
		DrawVec3(ubo->Direction, "Direction");
		DrawVec3(ubo->Ambient, "Ambient");
		DrawVec3(ubo->Diffuse, "Diffuse");
		DrawVec3(ubo->Specular, "Specular");
	}

	void PointLightComponent::ImGuiDraw() {
		PointLight::PointLightUBO* ubo = Light->GetUBO(0);
		DrawVec3(ubo->Position, "Position");
		DrawVec3(ubo->Ambient, "Ambient");
		DrawVec3(ubo->Diffuse, "Diffuse");
		DrawVec3(ubo->Specular, "Specular");
		ImGui::DragFloat("Constant", &ubo->Constant, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::DragFloat("Linear", &ubo->Linear, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::DragFloat("Quadratic", &ubo->Quadratic, 0.1f, 0.0f, 0.0f, "%.2f");
	}

	void SpotLightComponent::ImGuiDraw() {
		SpotLight::SpotLightUBO* ubo = Light->GetUBO();
		DrawVec3(ubo->Position, "Position");
		DrawVec3(ubo->Direction, "Direction");
		DrawVec3(ubo->Ambient, "Ambient");
		DrawVec3(ubo->Diffuse, "Diffuse");
		DrawVec3(ubo->Specular, "Specular");
		ImGui::DragFloat("CutOff", &ubo->CutOff, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::DragFloat("Outer CutOff", &ubo->OuterCutOff, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::DragFloat("Constant", &ubo->Constant, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::DragFloat("Linear", &ubo->Linear, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::DragFloat("Quadratic", &ubo->Quadratic, 0.1f, 0.0f, 0.0f, "%.2f");
	}
}