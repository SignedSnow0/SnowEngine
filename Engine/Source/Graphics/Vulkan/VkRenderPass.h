#pragma once
#include <optional>

#include "VkSurface.h"
#include "VkImage.h"
#include "Graphics/Rhi/RenderPass.h"

namespace SnowEngine
{
	class VkRenderPass : public RenderPass 
	{
	public:
		VkRenderPass(std::shared_ptr<const VkSurface> surface);
		VkRenderPass(u32 frameCount, u32 width, u32 height);

		u32 Width() const override;
		u32 Height() const override;

		vk::RenderPass RenderPass() const;
		const std::vector<std::unique_ptr<VkImage>>& Images() const;

		void Begin(const std::shared_ptr<CommandBuffer>& cmd) override;
		void End(const std::shared_ptr<CommandBuffer>& cmd) const override;

		void Resize(u32 width, u32 height);

	private:
		void CreateAttachments(vk::Format format, vk::ImageLayout layout);
		void CreateSubpasses();
		void CreateDependencies(vk::PipelineStageFlags pipelineStage, vk::AccessFlags access);
		void CreateRenderPass();
		void CreateFramebuffer(vk::ImageView view, u32 currentFrame);
		void CreateImage(u32 currentFrame);

		std::vector<vk::AttachmentDescription> mAttachments;
		std::vector<vk::SubpassDescription> mSubpasses;
		std::vector<vk::AttachmentReference> mReferences;
		std::vector<vk::SubpassDependency> mDependencies;
		std::vector<vk::Framebuffer> mFramebuffers;
		vk::RenderPass mRenderPass;
		std::shared_ptr<const VkSurface> mSurface;
		std::vector<std::unique_ptr<VkImage>> mImages;
		u32 mWidth, mHeight;
	};
}
