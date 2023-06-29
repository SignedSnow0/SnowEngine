#pragma once
#include <glm/glm.hpp>

#include "Core/Types.h"

namespace SnowEngine
{
	class Camera
	{
	public:
		void SetPosition(const glm::vec3& position);
		void SetRotation(const glm::vec3& rotation);
		void SetUp(const glm::vec3& up);
		void SetFov(f32 fov);
		void SetNear(f32 near);
		void SetFar(f32 far);
		void SetAspectRatio(f32 aspectRatio);

		glm::vec3 Position() const;
		glm::vec3 Rotation() const;
		glm::vec3 Up() const;
		f32 Fov() const;
		f32 Near() const;
		f32 Far() const;
		f32 AspectRatio() const;

		glm::mat4 View() const;
		glm::mat4 Projection() const;

	private:
		glm::vec3 mPosition{ 0.0f, 0.0f, 0.0f };
		glm::vec3 mRotation{ 1.0f, 0.0f, 0.0f };
		glm::vec3 mUp{ 0.0f, 1.0f, 0.0f };
		f32 mFov{ 90.0f };
		f32 mNear{ 0.001f };
		f32 mFar{ 1000.0f };
		f32 mAspectRatio{ 16.0f / 9.0f };
	};

	class CameraController
	{
	public:
		virtual void Update(f32 dt) = 0;

		virtual glm::mat4 View() const;
		virtual glm::mat4 Projection() const;

	protected:
		Camera mCamera;
	};

	class FirstPersonCamera : public CameraController
	{
	public:
		void Update(f32 dt) override;

	private:
		f32 mSpeed{ 1.0f };
		f32 mSensitivity{ 1.0f };
	};
}