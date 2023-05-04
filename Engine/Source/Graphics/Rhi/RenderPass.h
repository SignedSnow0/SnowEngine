#pragma once
#include <memory>

#include "Surface.h"

namespace SnowEngine
{
	class RenderPass
	{
	public:
		static std::shared_ptr<RenderPass> Create(const std::shared_ptr<const Surface>& surface);
		virtual ~RenderPass() = default;

		virtual void Begin() const = 0;
		virtual void End() const = 0;
	};
}
