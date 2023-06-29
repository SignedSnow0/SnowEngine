#include "Camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include "Core/Input.h"

namespace SnowEngine 
{
	void Camera::SetPosition(const glm::vec3& position) { mPosition = position; }
	void Camera::SetRotation(const glm::vec3& rotation) { mRotation = glm::normalize(rotation); }
	void Camera::SetUp(const glm::vec3& up) { mUp = up; }
	void Camera::SetFov(f32 fov) { mFov = fov; }
	void Camera::SetNear(f32 near) { mNear = near; }
	void Camera::SetFar(f32 far) { mFar = far; }
	void Camera::SetAspectRatio(f32 aspectRatio) { mAspectRatio = aspectRatio; }

	glm::vec3 Camera::Position() const { return mPosition; }
	glm::vec3 Camera::Rotation() const { return mRotation; }
	glm::vec3 Camera::Up() const { return mUp; }
	f32 Camera::Fov() const { return mFov; }
	f32 Camera::Near() const { return mNear; }
	f32 Camera::Far() const {	return mFar; }
	f32 Camera::AspectRatio() const { return mAspectRatio; }

	glm::mat4 Camera::View() const
	{
		//TODO: rotation on unit sphere
		return glm::lookAt(mPosition, mPosition + mRotation, mUp);
	}

	glm::mat4 Camera::Projection() const
	{
		auto proj = glm::perspective(mFov, mAspectRatio, mNear, mFar);
		proj[1][1] *= -1;
		return proj;
	}

	glm::mat4 CameraController::View() const { return mCamera.View(); }

	glm::mat4 CameraController::Projection() const { return mCamera.Projection(); }
	
	void FirstPersonCamera::Update(f32 dt)
	{
		auto cameraFront = mCamera.Rotation();
		auto cameraUp = mCamera.Up();
		auto cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

		if (Input::IsKeyPressed(Key::W))
			mCamera.SetPosition(mCamera.Position() + cameraFront * mSpeed * dt);
		if (Input::IsKeyPressed(Key::S))
			mCamera.SetPosition(mCamera.Position() - cameraFront * mSpeed * dt);
		if (Input::IsKeyPressed(Key::A))
			mCamera.SetPosition(mCamera.Position() - cameraRight * mSpeed * dt);
		if (Input::IsKeyPressed(Key::D))
			mCamera.SetPosition(mCamera.Position() + cameraRight * mSpeed * dt);
		if (Input::IsKeyPressed(Key::Space))
			mCamera.SetPosition(mCamera.Position() + cameraUp * mSpeed * dt);
		if (Input::IsKeyPressed(Key::LeftShift))
			mCamera.SetPosition(mCamera.Position() - cameraUp * mSpeed * dt);
	}
}