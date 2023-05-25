#include "Mesh.h"

namespace SnowEngine
{
	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<u32>& indices, const u32 frameCount)
	{
		mVertexBuffer = VertexBuffer::Create(vertices.data(), static_cast<u32>(vertices.size()));
		mIndexBuffer = IndexBuffer::Create(indices.data(), static_cast<u32>(indices.size()));

		if (std::shared_ptr<Shader> shader; Shader::GetShader("default", shader))
			mTransformDescriptorSet = DescriptorSet::Create(shader, 1, frameCount);//TODO: better way
	}

	void Mesh::SetTransform(const glm::mat4& transform, const u32 frameIndex) const
	{
		mTransformDescriptorSet->SetUniform("Transform", &transform, frameIndex);
	}

	void Mesh::SetAlbedo(const std::shared_ptr<Image>& albedo) const
	{
		mTransformDescriptorSet->SetImage("albedo", albedo);
	}

	void Mesh::Draw(const std::shared_ptr<Pipeline>& pipeline, const std::shared_ptr<CommandBuffer>& cmd, const u32 currentFrame) const
	{
		pipeline->BindDescriptorSet(mTransformDescriptorSet.get(), currentFrame, cmd);

		mVertexBuffer->Bind(cmd);

		if (mIndexBuffer)
		{
			mIndexBuffer->Bind(cmd);
			mIndexBuffer->Draw(cmd);

			return;
		}

		mVertexBuffer->Draw(cmd);
	}
}
