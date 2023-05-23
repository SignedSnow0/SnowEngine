﻿#include "Editor.h"

#include <chrono>
#include <imgui.h>

int main()
{
	SnowEditor::Editor editor{};
	editor.Run();

	return 0;
}

namespace SnowEditor
{
	Editor::Editor()
	{
		SnowEngine::GraphicsCore::Init();

		mWindow = SnowEngine::Window::Create("SnowEngine", 1920, 1080);
		mSurface = SnowEngine::Surface::Create(mWindow);

		mScene = std::make_shared<SnowEngine::Scene>();

		mSceneRenderer = std::make_shared<SnowEngine::SceneRenderer>(mSurface);
		mSceneRenderer->SetScene(mScene);

		mGui = SnowEngine::Gui::Create(mSurface, mSceneRenderer->GetRenderPass());

		mSceneView.SetScene(mScene);
		mSceneView.SetEntityView(&mEntityView);
	}

	Editor::~Editor()
	{
		SnowEngine::GraphicsCore::Shutdown();
	}

	void Editor::Run()
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto lastTime = std::chrono::high_resolution_clock::now();
		while (!mWindow->Closing())
		{
			currentTime = std::chrono::high_resolution_clock::now();
			const f32 time = std::chrono::duration<f32, std::chrono::seconds::period>(currentTime - lastTime).count();

			mSurface->Begin();

			mSceneRenderer->Draw(mSurface);
			auto& sceneBuffer = mSceneRenderer->GetCommandBuffer();

			mGui->Begin(sceneBuffer);

			ImGui::ShowStyleEditor();
			ImGui::ShowDemoWindow();

			mSceneView.Draw();

			mEntityView.Draw();

			mGui->End(sceneBuffer);

			sceneBuffer->End(mSurface->CurrentFrame());

			sceneBuffer->Submit(mSurface->CurrentFrame(), mSurface);

			mSurface->End(sceneBuffer);

			SnowEngine::Window::Update();

			lastTime = currentTime;
		}
	}
}
