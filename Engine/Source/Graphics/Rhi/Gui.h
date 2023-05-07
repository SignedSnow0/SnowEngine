#pragma once
#include <memory>

#include "RenderPass.h"
#include "Surface.h"

namespace SnowEngine
{
	class Gui
	{
	public:
		static std::shared_ptr<Gui> Create(const std::shared_ptr<const Surface>& surface, const std::shared_ptr<RenderPass> &scene);
		virtual ~Gui() = default;

		virtual void Begin() const = 0;
		virtual void End() = 0;
	};
}
