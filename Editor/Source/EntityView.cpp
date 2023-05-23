#include "EntityView.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace ImGui
{
	ImVector<std::pair<ImDrawListSplitter*, bool>> sGroupPanelStack;

	bool BeginGroupPanel(const char* name, const ImVec2& size)
	{
		//draw foreground ad background separately
		const auto splitter = new ImDrawListSplitter;
		splitter->Split(GetWindowDrawList(), 2);
		splitter->SetCurrentChannel(GetWindowDrawList(), 1);

		BeginGroup();

		ImVec2 effectiveSize = size;
		if (size.x == 0.0f)
			effectiveSize.x = ImGui::GetContentRegionAvail().x;
		else
			effectiveSize.x = size.x;
		Dummy(ImVec2(effectiveSize.x, 0.0f));

		//remove header color
		PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

		const bool open = CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen);

		PopStyleColor(3);

		//right padding
		Indent(GetStyle().WindowPadding.x);

		sGroupPanelStack.push_back({ splitter, open });
		return open;
    }

    void EndGroupPanel()
    {
		const auto& [splitter, open] = sGroupPanelStack.back();

		//bottom padding
		Dummy(ImVec2(0.0f, open ? GetStyle().WindowPadding.y : GetStyle().FramePadding.y));

    	EndGroup();
		const auto minSize = GetItemRectMin();
		const auto maxSize = GetItemRectMax();

		splitter->SetCurrentChannel(GetWindowDrawList(), 0);

		GetWindowDrawList()->AddRectFilled(minSize, maxSize, GetColorU32(ImGuiCol_ChildBg), GetStyle().FrameRounding);

		sGroupPanelStack.back().first->Merge(GetWindowDrawList());
		sGroupPanelStack.pop_back();
		delete splitter;
	}
}


namespace SnowEditor
{
	void EntityView::SetEntity(const SnowEngine::Entity& entity) { mEntity = entity; }

	void EntityView::Draw()
	{
		if (ImGui::Begin("Components") && mEntity.IsValid())
		{
			DrawComponent<SnowEngine::Tag>("Tag", [](SnowEngine::Tag& tag)
			{
				char name[256];
				strcpy_s(name, 256, tag.Name.c_str());

				if (ImGui::InputText("##TagInput", name, 256))
					tag.Name = name;
			});

			DrawComponent<SnowEngine::Transform>("Transform", [](SnowEngine::Transform& transform)
			{
				ImGui::DragFloat3("Position", &transform.Position.x, 0.1f);
				ImGui::DragFloat3("Rotation", &transform.Rotation.x, 0.1f);
				ImGui::DragFloat3("Scale", &transform.Scale.x, 0.1f);
			});
		}
		ImGui::End();
	}

	
}
