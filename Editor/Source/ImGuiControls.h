#pragma once
#include <string>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <glm/glm.hpp>

namespace ImGui
{
	bool BeginGroupPanel(const char* name, const ImVec2& size = ImVec2(0.0f, 0.0f));
	void EndGroupPanel();

	bool Vec3Slider(const std::string& name, glm::vec3& value, const glm::vec3& min, const glm::vec3& max);

	bool RoundedButton(const std::string& label, ImDrawFlags rounding = 0);
	bool RoundedDrag(const std::string& label, float* val, ImDrawFlags rounding = 0);

	void ToggleButton(const std::string& label, bool* pressed, const ImVec2& size = ImVec2(0, 0), ImDrawFlags rounding = ImDrawFlags_RoundCornersAll);
}
