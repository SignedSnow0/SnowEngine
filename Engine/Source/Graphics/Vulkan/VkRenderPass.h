#pragma once
#include "VkSurface.h"
#include "Graphics/Rhi/RenderPass.h"

namespace SnowEngine
{
	class VkRenderPass : public RenderPass 
	{
	public:
		VkRenderPass(std::shared_ptr<const VkSurface> surface);

		vk::RenderPass RenderPass() const;

		void Begin() const override;
		void End() const override;

	private:
		void CreateAttachments();
		void CreateSubpasses();
		void CreateDependencies();
		void CreateRenderPass();
		void CreateFramebuffers();

		std::vector<vk::AttachmentDescription> mAttachments;
		std::vector<vk::SubpassDescription> mSubpasses;
		std::vector<vk::AttachmentReference> mReferences;
		std::vector<vk::SubpassDependency> mDependencies;
		std::vector<vk::Framebuffer> mFramebuffers;
		vk::RenderPass mRenderPass;
		std::shared_ptr<const VkSurface> mSurface;
	};
}
