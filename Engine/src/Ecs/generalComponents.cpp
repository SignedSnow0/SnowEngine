#include "generalComponents.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <imguiLib/imgui.h>
#include <imgui_internal.h>
#include <entt.hpp>
#include <nfd.h>

namespace SnowEngine {
	void TransformComponent::DrawVec3(glm::vec3& value, const std::string label) {
		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 100.0f);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.0f, 0.0f, 0.0f, 0.2f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.0f, 0.0f, 0.0f, 0.5f });
		if (ImGui::Button(label.c_str()))
			value = glm::vec3(0.0f);
		
		ImGui::PopStyleColor(3);
		ImGui::NextColumn();
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::DragFloat("##X", &value.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &value.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::SameLine();
		ImGui::DragFloat("##Z", &value.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	void TransformComponent::ImGuiDraw() {
		DrawVec3(Translation, "Translation");
		Rotation = glm::degrees(Rotation);
		DrawVec3(Rotation, "Rotation");
		Rotation = glm::radians(Rotation);
		DrawVec3(Scale, "Scale");
	}

	glm::mat4 TransformComponent::GetTransform() {
		glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
		return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
	}

	void ModelComponent::ImGuiDraw() {
		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::Text(model != nullptr ? model->GetName().c_str() : "No model loaded!");
		ImGui::SameLine();
		if (ImGui::Button("Select a file")) {
			nfdpathset_t resPath;
			nfdresult_t result = NFD_OpenDialogMultiple("obj,gltf", nullptr, &resPath);
			if (result == NFD_OKAY) {
				for (size_t i = 0; i < NFD_PathSet_GetCount(&resPath); ++i) {
					nfdchar_t* path = NFD_PathSet_GetPath(&resPath, i);
					model = new Model(Device::Get(), path);
				}
				NFD_PathSet_Free(&resPath);
			}
		}
		ImGui::PopItemWidth();
	}
}