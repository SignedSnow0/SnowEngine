#pragma once
#include <memory>
#include <glm/glm.hpp>

#include "CommandBuffer.h"
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

		virtual void Bind(const std::shared_ptr<CommandBuffer>& cmd) const = 0;
		virtual void Draw(const std::shared_ptr<CommandBuffer>& cmd) const = 0;
	};

	class IndexBuffer
	{
	public:
		static std::shared_ptr<IndexBuffer> Create(const u32* indices, u32 indexCount);
		virtual ~IndexBuffer() = default;

		virtual void Bind(const std::shared_ptr<CommandBuffer>& cmd) const = 0;
		virtual void Draw(const std::shared_ptr<CommandBuffer>& cmd) const = 0;
	};

	class StorageBuffer
	{
	public:
		static std::shared_ptr<StorageBuffer> Create(u32 size);
		virtual ~StorageBuffer() = default;

		virtual void SetData(const std::shared_ptr<StorageBuffer>& other) const = 0;
		virtual void SetData(const void* data) const = 0;
	};
}
