#include "SceneView.h"
#include <imgui.h>

namespace SnowEditor
{
	void SceneView::SetScene(const std::shared_ptr<SnowEngine::Scene>& scene) { mScene = scene; }

	void SceneView::Draw()
	{
		if (!mScene)
			return;

		if (ImGui::Begin("Entities"))
		{
			if(ImGui::Button("Create Entity"))
			{
				SnowEngine::Entity e = mScene->CreateEntity();
				e.AddComponent<SnowEngine::Component::Tag>("New entity");
				e.AddComponent<SnowEngine::Component::Transform>();
				e.AddComponent<SnowEngine::Component::Mesh>();//TODO: tmp
			}

			u32 id{ 0 };
			mScene->ExecuteSystem([&](const SnowEngine::Entity& e)
			{
				const auto tag = e.GetComponents<SnowEngine::Component::Tag>();
				const std::string label = tag.Name + "##" + std::to_string(id);
				if (ImGui::Selectable(label.c_str()))
				{
					if (mEntityView)
						mEntityView->SetEntity(e);
				}

				id++;
			});
		}
		ImGui::End();
	}
}
