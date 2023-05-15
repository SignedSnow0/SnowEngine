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
		VkRenderPass(std::shared_ptr<const VkSurface> surface, b8 depth);
		VkRenderPass(u32 frameCount, u32 width, u32 height, b8 depth);

		u32 Width() const override;
		u32 Height() const override;

		vk::RenderPass RenderPass() const;
		const std::vector<std::unique_ptr<VkImage>>& Images() const;
<<<<<<< HEAD
		b8 HasDepth() const;
=======
>>>>>>> 82bf1ed14055334cdce1014d7850da732bbad42a

		void Begin(const std::shared_ptr<CommandBuffer>& cmd) override;
		void End(const std::shared_ptr<CommandBuffer>& cmd) const override;

		void Resize(u32 width, u32 height);

	private:
		void CreateAttachments(vk::Format format, vk::ImageLayout layout);
		void CreateSubpasses();
		void CreateDependencies(vk::PipelineStageFlagBits pipelineStage, vk::AccessFlags access);
		void CreateRenderPass();
		void CreateFramebuffer(const std::vector<vk::ImageView>& views, u32 currentFrame);
		void CreateImage(u32 currentFrame);
		void CreateDepthImage(u32 currentFrame);

		std::vector<vk::AttachmentDescription> mAttachments;
		std::vector<vk::SubpassDescription> mSubpasses;
		std::vector<vk::AttachmentReference> mReferences;
		std::vector<vk::SubpassDependency> mDependencies;
		std::vector<vk::Framebuffer> mFramebuffers;
		vk::RenderPass mRenderPass;
		std::shared_ptr<const VkSurface> mSurface;
		std::vector<std::unique_ptr<VkImage>> mImages;
		std::vector<std::unique_ptr<VkImage>> mDepthImages;
		u32 mWidth, mHeight;
		b8 mHasDepth;
	};
}
