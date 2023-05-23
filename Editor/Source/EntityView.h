#pragma once
#include <SnowEngine.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

namespace ImGui
{
	bool BeginGroupPanel(const char* name, const ImVec2& size = ImVec2(0.0f, 0.0f));
	void EndGroupPanel();
}

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
