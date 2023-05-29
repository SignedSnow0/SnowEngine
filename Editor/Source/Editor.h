#pragma once
#include <SnowEngine.h>

#include "EntityView.h"
#include "LogView.h"
#include "SceneView.h"

namespace SnowEditor
{
	class Editor
	{
	public:
		Editor();
		~Editor();

		void Run();

	private:
		std::shared_ptr<SnowEngine::Scene> mScene{ nullptr };
		std::shared_ptr<SnowEngine::Window> mWindow{ nullptr };
		std::shared_ptr<SnowEngine::Surface> mSurface{ nullptr };
		
		std::shared_ptr<SnowEngine::SceneRenderer> mSceneRenderer{ nullptr };
		std::shared_ptr<SnowEngine::Gui> mGui{ nullptr };

		SceneView* mSceneView;
		EntityView* mEntityView;
		LogView* mLogView;
	};
}
