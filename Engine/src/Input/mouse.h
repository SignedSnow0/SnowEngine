#pragma once
#include <utility>

namespace SnowEngine {
	class Mouse {
		friend class Window;
	public:
		static bool IsButtonPessed(int button);
		static std::pair<float, float> GetPosition();
		static std::pair<int, int> GetWheelDelta();

	private:
		static void Init();
		static void SetButton(int button, bool value);
		static void SetPosition(std::pair<float, float> position);
		static void SetWheelDelta(std::pair<int, int> delta);
	};

#define SNOW_MOUSE_BUTTON_1         0
#define SNOW_MOUSE_BUTTON_2         1
#define SNOW_MOUSE_BUTTON_3         2
#define SNOW_MOUSE_BUTTON_4         3
#define SNOW_MOUSE_BUTTON_5         4
#define SNOW_MOUSE_BUTTON_6         5
#define SNOW_MOUSE_BUTTON_7         6
#define SNOW_MOUSE_BUTTON_8         7
#define SNOW_MOUSE_BUTTON_LAST      SNOW_MOUSE_BUTTON_8
#define SNOW_MOUSE_BUTTON_LEFT      SNOW_MOUSE_BUTTON_1
#define SNOW_MOUSE_BUTTON_RIGHT     SNOW_MOUSE_BUTTON_2
#define SNOW_MOUSE_BUTTON_MIDDLE    SNOW_MOUSE_BUTTON_3
}