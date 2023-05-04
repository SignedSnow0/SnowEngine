#include "VkRenderPass.h"

#include "VkCore.h"

namespace SnowEngine
{
	VkRenderPass::VkRenderPass(std::shared_ptr<const VkSurface> surface)
		: mSurface{ std::move(surface) }
	{
		CreateAttachments();
		CreateSubpasses();
		CreateDependencies();
		CreateRenderPass();
		CreateFramebuffers();
	}

	vk::RenderPass VkRenderPass::RenderPass() const { return mRenderPass; }

	void VkRenderPass::Begin() const
	{
		const vk::ClearValue clearColor{ vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} } };

		vk::RenderPassBeginInfo beginInfo{};
		beginInfo.renderPass = mRenderPass;
		beginInfo.framebuffer = mFramebuffers[VkSurface::BoundSurface()->GetCurrentFrame()];
		beginInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
		beginInfo.renderArea.extent = vk::Extent2D{ VkSurface::BoundSurface()->GetWidth(), VkSurface::BoundSurface()->GetHeight() };
		beginInfo.clearValueCount = 1;
		beginInfo.pClearValues = &clearColor;

		VkSurface::BoundSurface()->GetCommandBuffer().beginRenderPass(beginInfo, vk::SubpassContents::eInline);
	}

	void VkRenderPass::End() const
	{
		VkSurface::BoundSurface()->GetCommandBuffer().endRenderPass();
	}

	void VkRenderPass::CreateAttachments()
	{
		vk::AttachmentDescription& colorAttachment{ mAttachments.emplace_back() };
		colorAttachment.format = mSurface->GetFormat();
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

		vk::AttachmentReference& colorAttachmentRef{ mReferences.emplace_back() };
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
	}

	void VkRenderPass::CreateSubpasses()
	{
		vk::SubpassDescription& subpass{ mSubpasses.emplace_back() };
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &mReferences[0];
	}

	void VkRenderPass::CreateDependencies()
	{
		vk::SubpassDependency& dependency{ mDependencies.emplace_back() };
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	}

	void VkRenderPass::CreateRenderPass()
	{
		vk::RenderPassCreateInfo createInfo{};
		createInfo.attachmentCount = static_cast<u32>(mAttachments.size());
		createInfo.pAttachments = mAttachments.data();
		createInfo.subpassCount = static_cast<u32>(mSubpasses.size());
		createInfo.pSubpasses = mSubpasses.data();
		createInfo.dependencyCount = static_cast<u32>(mDependencies.size());
		createInfo.pDependencies = mDependencies.data();

		mRenderPass = VkCore::Get()->Device().createRenderPass(createInfo);
	}

	void VkRenderPass::CreateFramebuffers()
	{
		mFramebuffers.reserve(mSurface->GetViews().size());

		for (auto view : mSurface->GetViews())
		{
			vk::FramebufferCreateInfo createInfo{};
			createInfo.renderPass = mRenderPass;
			createInfo.attachmentCount = 1;
			createInfo.pAttachments = &view;
			createInfo.width = mSurface->GetWidth();
			createInfo.height = mSurface->GetHeight();
			createInfo.layers = 1;

			mFramebuffers.emplace_back(VkCore::Get()->Device().createFramebuffer(createInfo));
		}
	}
}
