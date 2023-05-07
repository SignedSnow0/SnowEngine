#include "VkRenderPass.h"

#include "VkCore.h"

namespace SnowEngine
{
	VkRenderPass::VkRenderPass(std::shared_ptr<const VkSurface> surface)
		: mSurface{ std::move(surface) }, mWidth{ mSurface->GetWidth() }, mHeight{ mSurface->GetHeight() }
	{
		CreateAttachments(mSurface->GetFormat(), vk::ImageLayout::ePresentSrcKHR);
		CreateSubpasses();
		CreateDependencies(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
		CreateRenderPass();

		CreateFramebuffers(mSurface->GetViews());
	}

	VkRenderPass::VkRenderPass(const u32 frameCount, const u32 width, const u32 height)
		: mWidth{ width }, mHeight{ height }
	{
		CreateAttachments(vk::Format::eB8G8R8A8Srgb, vk::ImageLayout::eShaderReadOnlyOptimal);
		CreateSubpasses();
		CreateDependencies(vk::PipelineStageFlagBits::eFragmentShader, vk::AccessFlagBits::eShaderRead);
		CreateRenderPass();
		CreateImages(frameCount);

		std::vector<vk::ImageView> views{};
		views.reserve(mImages.size());
		for (const auto& image : mImages)
			views.emplace_back(image->GetView());

		CreateFramebuffers(views);
	}

	vk::RenderPass VkRenderPass::RenderPass() const { return mRenderPass; }

	const std::vector<std::unique_ptr<VkImage>>& VkRenderPass::Images() const { return mImages; }

	u32 VkRenderPass::Width() const { return mWidth; }

	u32 VkRenderPass::Height() const { return mHeight; }

	void VkRenderPass::Resize(const u32 width, const u32 height)
	{
		mWidth = width;
		mHeight = height;

		mFramebuffers.clear();

		if (mSurface)
		{
			CreateFramebuffers(mSurface->GetViews());
			return;
		}

		const u32 frameCount = static_cast<u32>(mImages.size());
		mImages.clear();

		CreateImages(frameCount);
		std::vector<vk::ImageView> views{};
		views.reserve(mImages.size());
		for (const auto& image : mImages)
			views.emplace_back(image->GetView());

		CreateFramebuffers(views);
	}

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

	void VkRenderPass::CreateAttachments(const vk::Format format, const vk::ImageLayout layout)
	{
		vk::AttachmentDescription& colorAttachment{ mAttachments.emplace_back() };
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.format = format;
		colorAttachment.finalLayout = layout;

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

	void VkRenderPass::CreateDependencies(const vk::PipelineStageFlags pipelineStage, const vk::AccessFlags access)
	{
		vk::SubpassDependency& dependency{ mDependencies.emplace_back() };
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcStageMask = pipelineStage;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstAccessMask = access;
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

	void VkRenderPass::CreateFramebuffers(const std::vector<vk::ImageView>& views)
	{
		mFramebuffers.reserve(views.size());

		for (const auto& view : views)
		{
			vk::FramebufferCreateInfo createInfo{};
			createInfo.renderPass = mRenderPass;
			createInfo.attachmentCount = 1;
			createInfo.pAttachments = &view;
			createInfo.width = mWidth;
			createInfo.height = mHeight;
			createInfo.layers = 1;

			mFramebuffers.emplace_back(VkCore::Get()->Device().createFramebuffer(createInfo));
		}
	}

	void VkRenderPass::CreateImages(const u32 frameCount)
	{
		for (u32 i = 0; i < frameCount; i++)
			mImages.emplace_back(std::make_unique<VkImage>(mWidth, mHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal));
	}
}
