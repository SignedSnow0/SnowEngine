#pragma once
#include <SnowEngine.h>

#include "ImGuiControls.h"

namespace SnowEditor
{
	class EntityView
	{
	public:
		void SetEntity(const SnowEngine::Entity& entity);

		void Draw();

	private:
		template<typename Comp>
		void DrawComponent(const std::string& name, std::function<void(Comp& comp)> func)
		{
			if (mEntity.HasComponents<Comp>())
			{
				if (ImGui::BeginGroupPanel(name.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 100.0f)))
				{
					auto& comp{ mEntity.GetComponents<Comp>() };
					func(comp);
				}

				ImGui::EndGroupPanel();
			}
		}

		SnowEngine::Entity mEntity;
	};
}
