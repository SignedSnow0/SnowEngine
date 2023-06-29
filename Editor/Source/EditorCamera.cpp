#include "EditorCamera.h"

namespace SnowEditor
{
	void EditorCamera::Update(f32 dt)
	{
		auto mousePos = SnowEngine::Input::GetMousePosition();
		auto mouseDelta = mousePos - mMousePosition;

		if (mouseDelta == glm::vec2{ 0.0f })
			return;

		if (SnowEngine::Input::IsMouseButtonPressed(SnowEngine::Button::Left))
		{
			if (SnowEngine::Input::IsKeyPressed(SnowEngine::Key::LeftShift))
			{
				auto cameraFront = mCamera.Rotation();
				auto cameraUp = mCamera.Up();
				auto cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

				//TODO: movement ambplified by distance of clicked object from eye
				glm::vec3 movement = cameraRight * mouseDelta.x * dt;
				movement += cameraUp * mouseDelta.y * dt;
				mCamera.SetPosition(mCamera.Position() + movement);
			}
			else
			{
				glm::vec3 newRotation = mCamera.Rotation() + glm::vec3{ 0.0f, mouseDelta.y * dt, mouseDelta.x * dt };
				if (newRotation.x > 1.5f)
					newRotation.x = 1.5f;
				else if (newRotation.x < -1.5f)
					newRotation.x = -1.5f;
				mCamera.SetRotation(newRotation);
			}
		}

		mMousePosition = mousePos;
	}
}