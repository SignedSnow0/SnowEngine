#pragma once
#include <SnowEngine.h>

namespace SnowEditor
{
	class EditorCamera : public SnowEngine::CameraController
	{
	public:
		void Update(f32 dt) override;
		
	private:
		glm::vec2 mMousePosition{};
	};
}