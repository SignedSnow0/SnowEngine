#include "Input.h"

namespace SnowEngine
{
	std::array<b8, 512> Input::sKeys{ false };
	std::array<b8, 16> Input::sButtons{ false };
	glm::vec2 Input::sMousePosition{ 0.0f, 0.0f };
	glm::vec2 Input::sMouseScroll{ 0.0f, 0.0f };

	b8 Input::IsKeyPressed(Key key) { return sKeys[static_cast<u32>(key)]; }

	b8 Input::IsMouseButtonPressed(Button button) { return sButtons[static_cast<u32>(button)]; }

	glm::vec2 Input::GetMouseScroll() { return sMouseScroll; }

	glm::vec2 Input::GetMousePosition() { return sMousePosition; }

	f32 Input::GetMouseX() { return sMousePosition.x; }

	f32 Input::GetMouseY() { return sMousePosition.y; }

	void Input::SetKey(Key key, const b8 pressed) { sKeys[static_cast<u32>(key)] = pressed; }

	void Input::SetButton(Button button, const b8 pressed) { sButtons[static_cast<u32>(button)] = pressed; }

	void Input::SetMousePosition(f32 x, f32 y) { sMousePosition = { x, y }; }

	void Input::SetMouseScroll(f32 x, f32 y) { sMouseScroll = { x, y }; }
}
