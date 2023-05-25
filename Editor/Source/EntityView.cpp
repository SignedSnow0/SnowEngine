#include "EntityView.h"

namespace SnowEditor
{
	void EntityView::SetEntity(const SnowEngine::Entity& entity) { mEntity = entity; }

	void EntityView::Draw()
	{
		if (ImGui::Begin("Components") && mEntity.IsValid())
		{
			DrawComponent<SnowEngine::Component::Tag>("Tag", [](SnowEngine::Component::Tag& tag)
			{
				char name[256];
				strcpy_s(name, 256, tag.Name.c_str());

				if (ImGui::InputText("##TagInput", name, 256))
					tag.Name = name;
			});

			DrawComponent<SnowEngine::Component::Transform>("Transform", [](SnowEngine::Component::Transform& transform)
			{
				ImGui::Vec3Slider("Position", transform.Position, glm::vec3(0.0f), glm::vec3(1.0f));
				ImGui::Vec3Slider("Rotation", transform.Rotation, glm::vec3(0.0f), glm::vec3(1.0f));
				ImGui::Vec3Slider("Scale   ", transform.Scale, glm::vec3(0.0f), glm::vec3(1.0f));
			});
		}
		ImGui::End();
	}

	
}
