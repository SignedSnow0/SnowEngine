#pragma once
#include <SnowEngine.h>

#include "EntityView.h"

namespace SnowEditor
{
	class SceneView
	{
	public:
		void SetScene(const std::shared_ptr<SnowEngine::Scene>& scene);
		void SetEntityView(EntityView* entityView) { mEntityView = entityView; }

		void Draw();

	private:
		std::shared_ptr<SnowEngine::Scene> mScene{ nullptr };

		EntityView* mEntityView{ nullptr };
	};
}
