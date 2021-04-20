#include "mouse.h"

namespace SnowEngine {
	static bool buttons[16];
	static std::pair<float, float> pos;
	static std::pair<int, int> wheelDelta;

	bool Mouse::IsButtonPessed(int button) {
		return buttons[button];
	}

	std::pair<float, float> Mouse::GetPosition() {
		return pos;
	}

	std::pair<int, int> Mouse::GetWheelDelta() {
		return wheelDelta;
	}

	void Mouse::Init() {
		for (bool button : buttons) button = false;
		pos = { 0.0f, 0.0f };
	}

	void Mouse::SetButton(int button, bool value) {
		buttons[button] = value;
	}

	void Mouse::SetPosition(std::pair<float, float> position) {
		pos = position;
	}
	void Mouse::SetWheelDelta(std::pair<int, int> delta) {
		wheelDelta = delta;
	}
}
