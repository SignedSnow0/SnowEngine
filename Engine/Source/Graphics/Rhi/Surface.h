#pragma once
#include <memory>

#include "Core/Window.h"

namespace SnowEngine
{
	class Surface
	{
	public:
		static std::shared_ptr<Surface> Create(std::shared_ptr<const Window> window);
		virtual ~Surface() = default;

		virtual u32 GetCurrentFrame() const = 0;

		virtual void Begin() = 0;
		virtual void End() = 0;
	};
}
