#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "Core/Types.h"

namespace SnowEngine
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Color;
		glm::vec2 Uv;
	};

	class VertexBuffer
	{
	public:
		static std::shared_ptr<VertexBuffer> Create(const Vertex* vertices, u32 vertexCount);
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Draw() const = 0;
	};

	class IndexBuffer
	{
	public:
		static std::shared_ptr<IndexBuffer> Create(const u32* indices, u32 indexCount);
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Draw() const = 0;
	};
}
