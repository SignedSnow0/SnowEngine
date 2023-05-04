#include "Buffers.h"

#include "Graphics/Vulkan/VkBuffers.h"

namespace SnowEngine
{
	std::shared_ptr<VertexBuffer> VertexBuffer::Create(const Vertex* vertices, u32 vertexCount)
	{
		return std::make_shared<VkVertexBuffer>(vertices, vertexCount);
	}

	std::shared_ptr<IndexBuffer> IndexBuffer::Create(const u32* indices, u32 indexCount)
	{
		return std::make_shared<VkIndexBuffer>(indices, indexCount);
	}
}
