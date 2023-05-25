#pragma once
#include "Rhi/Buffers.h"
#include "Rhi/DescriptorSet.h"
#include "Rhi/Pipeline.h"

namespace SnowEngine
{
	class Mesh
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<u32>& indices, u32 frameCount);

		void SetTransform(const glm::mat4& transform, u32 frameIndex) const;
		void SetAlbedo(const std::shared_ptr<Image>& albedo) const;

		void Draw(const std::shared_ptr<Pipeline>& pipeline, const std::shared_ptr<CommandBuffer>& cmd, u32 currentFrame) const;

	private:
		std::shared_ptr<VertexBuffer> mVertexBuffer{ nullptr };
		std::shared_ptr<IndexBuffer> mIndexBuffer{ nullptr };

		std::shared_ptr<DescriptorSet> mTransformDescriptorSet{ nullptr };
	};
}
