#include "ImGuiControls.h"

#include <utility>

#include <imgui_internal.h>

#include <SnowEngine.h>

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

	bool Vec3Slider(const std::string& name, glm::vec3& value, const glm::vec3& min, const glm::vec3& max)
	{
		PushID(GetID(name.c_str()));
		bool changed = false;

		TextUnformatted(name.c_str());
		const ImVec2 spacing{ GetStyle().ItemSpacing };

		ImVec2 totalSize{ GetContentRegionAvail() };
		totalSize.x -= 60.0f; //Buttons x 3
		totalSize.x -= GetStyle().FramePadding.x * 2.0f;
		totalSize.x -= CalcTextSize(name.c_str()).x;
		totalSize.x -= spacing.x;

		PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, spacing.y));
		PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

		SameLine();
		SetCursorPosX(GetCursorPosX() + spacing.x);

		PushStyleColor(ImGuiCol_Button, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::Red));
		PushStyleColor(ImGuiCol_ButtonHovered, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::RedHovered));
		PushStyleColor(ImGuiCol_ButtonActive, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::RedPressed));

		BeginGroup();

		RoundedButton("X", ImDrawFlags_RoundCornersLeft);
		PopStyleColor(3);
		SameLine();
		SetNextItemWidth(totalSize.x / 3.0f);
		changed |= DragFloat("##SX", &value.x);
		SameLine();
		PushStyleColor(ImGuiCol_Button, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::Green));
		PushStyleColor(ImGuiCol_ButtonHovered, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::GreenHovered));
		PushStyleColor(ImGuiCol_ButtonActive, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::GreenPressed));
		Button("Y", ImVec2(20.0f, 0.0f));
		PopStyleColor(3);
		SameLine();
		SetNextItemWidth(totalSize.x / 3.0f);
		changed |= DragFloat("##SY", &value.y);
		SameLine();
		PushStyleColor(ImGuiCol_Button, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::Blue));
		PushStyleColor(ImGuiCol_ButtonHovered, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::BlueHovered));
		PushStyleColor(ImGuiCol_ButtonActive, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::BluePressed));
		Button("Z", ImVec2(20.0f, 0.0f));
		PopStyleColor(3);
		SameLine();
		SetNextItemWidth(totalSize.x / 3.0f);
		changed |= RoundedDrag("##Z", &value.z, ImDrawFlags_RoundCornersRight);

		EndGroup();

		//GetWindowDrawList()->AddRect(GetItemRectMin(), GetItemRectMax(), GetColorU32(ImGuiCol_PopupBg), 4.0f);

		PopStyleVar(2);

		PopID();
		return changed;
	}

	bool RoundedButton(const std::string& label, const ImDrawFlags rounding)
	{
		ImDrawListSplitter splitter{};
		splitter.Split(GetWindowDrawList(), 2);
		splitter.SetCurrentChannel(GetWindowDrawList(), 1);

		const ImVec2 startPos = GetCursorPos();

		SetCursorPos(startPos + GetStyle().FramePadding);
		Text(label.c_str());

		SetCursorPos(startPos);
		const bool val = InvisibleButton("##RoundedButton", ImVec2(20.0f, 20.0f));
		const bool hovered = IsItemHovered();
		const bool active = IsItemActive();

		splitter.SetCurrentChannel(GetWindowDrawList(), 0);
		if (active)
			GetWindowDrawList()->AddRectFilled(GetItemRectMin(), GetItemRectMax(), GetColorU32(ImGuiCol_ButtonActive), 4.0f, rounding);
		else if (hovered)
			GetWindowDrawList()->AddRectFilled(GetItemRectMin(), GetItemRectMax(), GetColorU32(ImGuiCol_ButtonHovered), 4.0f, rounding);
		else
			GetWindowDrawList()->AddRectFilled(GetItemRectMin(), GetItemRectMax(), GetColorU32(ImGuiCol_Button), 4.0f, rounding);

		splitter.Merge(GetWindowDrawList());

		return val;
	}

	bool RoundedDrag(const std::string& label, float* val, const ImDrawFlags rounding)
	{
		ImDrawListSplitter splitter{};
		splitter.Split(GetWindowDrawList(), 2);
		splitter.SetCurrentChannel(GetWindowDrawList(), 1);

		PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

		const bool changed = DragFloat(label.c_str(), val);

		PopStyleColor(3);

		const bool hovered = IsItemHovered();
		const bool active = IsItemActive();

		splitter.SetCurrentChannel(GetWindowDrawList(), 0);
		if (active)
			GetWindowDrawList()->AddRectFilled(GetItemRectMin(), GetItemRectMax(), GetColorU32(ImGuiCol_FrameBgActive), 4.0f, rounding);
		else if (hovered)
			GetWindowDrawList()->AddRectFilled(GetItemRectMin(), GetItemRectMax(), GetColorU32(ImGuiCol_FrameBgHovered), 4.0f, rounding);
		else
			GetWindowDrawList()->AddRectFilled(GetItemRectMin(), GetItemRectMax(), GetColorU32(ImGuiCol_FrameBg), 4.0f, rounding);

		splitter.Merge(GetWindowDrawList());

		return changed;
	}
}
