#include "VkRenderPass.h"

#include "VkCore.h"
#include "VkCommandBuffer.h"

namespace SnowEngine
{
	VkRenderPass::VkRenderPass(std::shared_ptr<const VkSurface> surface, const b8 depth)
		: mSurface{ std::move(surface) }, mWidth{ mSurface->Width() }, mHeight{ mSurface->Height() }, mHasDepth{ depth }
	{
		CreateAttachments(mSurface->Format(), vk::ImageLayout::ePresentSrcKHR);
		CreateSubpasses();
		CreateDependencies(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite);
		CreateRenderPass();

		if (mHasDepth)
		{
			mDepthImages.resize(mSurface->Views().size());
			for (u32 i = 0; i < mSurface->Views().size(); i++)
				CreateDepthImage(i);
		}

		mFramebuffers.resize(mSurface->Views().size());
		for (u32 i = 0; i < mSurface->Views().size(); i++)
		{
			std::vector<vk::ImageView> views{ mSurface->Views()[i] };
			if (mHasDepth)
				views.emplace_back(mDepthImages[i]->View());

			CreateFramebuffer(views, i);
		}
	}

	VkRenderPass::VkRenderPass(const u32 frameCount, const u32 width, const u32 height, const b8 depth)
		: mWidth{ width }, mHeight{ height }, mHasDepth{ depth }
	{
		CreateAttachments(vk::Format::eB8G8R8A8Srgb, vk::ImageLayout::eShaderReadOnlyOptimal);
		CreateSubpasses();
		CreateDependencies(vk::PipelineStageFlagBits::eFragmentShader, vk::AccessFlagBits::eShaderRead);
		CreateRenderPass();

		if (mHasDepth)
			mDepthImages.resize(frameCount);

		mImages.resize(frameCount);
		mFramebuffers.resize(frameCount);
		for (u32 i = 0; i < frameCount; i++)
		{
			CreateImage(i);

			std::vector<vk::ImageView> views{ mImages[i]->View() };
			if (mHasDepth)
			{
				CreateDepthImage(i);
				views.emplace_back(mDepthImages[i]->View());
			}

			CreateFramebuffer(views, i);
		}
	}

	vk::RenderPass VkRenderPass::RenderPass() const { return mRenderPass; }

	const std::vector<std::unique_ptr<VkImage>>& VkRenderPass::Images() const { return mImages; }

	b8 VkRenderPass::HasDepth() const { return mHasDepth; }

	u32 VkRenderPass::Width() const { return mWidth; }

	u32 VkRenderPass::Height() const { return mHeight; }

	void VkRenderPass::Resize(const u32 width, const u32 height)
	{
		mWidth = width;
		mHeight = height;

		VkSurface::BoundSurface()->SubmitPostFrameQueue([this](const u32 frameIndex)
		{
			VkCore::Get()->Device().destroyFramebuffer(mFramebuffers[frameIndex]);
		
			if (mSurface)
			{
				std::vector<vk::ImageView> views{ mSurface->Views()[frameIndex] };
				if (mHasDepth)
				{
					CreateDepthImage(frameIndex);
					views.emplace_back(mDepthImages[frameIndex]->View());
				}

				CreateFramebuffer(views, frameIndex);

				return;
			}

			mImages[frameIndex].reset();

			CreateImage(frameIndex);
			std::vector<vk::ImageView> views{ mImages[frameIndex]->View() };
			if (mHasDepth)
			{
				CreateDepthImage(frameIndex);
				views.emplace_back(mDepthImages[frameIndex]->View());
			}
			CreateFramebuffer(views, frameIndex);
		});
	}

	void VkRenderPass::Begin(const std::shared_ptr<CommandBuffer>& cmd)
	{
		const auto& vkCmd = std::static_pointer_cast<VkCommandBuffer>(cmd);

		if (mSurface && (mWidth != mSurface->Width() || mHeight != mSurface->Height()))
		{
			mWidth = mSurface->Width();
			mHeight = mSurface->Height();

			for (u32 i{ 0 }; i < mSurface->Views().size(); i++)
			{
				VkCore::Get()->Device().destroyFramebuffer(mFramebuffers[i]);

				std::vector<vk::ImageView> views{ mSurface->Views()[i] };
				if (mHasDepth)
				{
					CreateDepthImage(i);
					views.emplace_back(mDepthImages[i]->View());
				}

				CreateFramebuffer(views, i);
			}
		}

		std::vector<vk::ClearValue> clearColors{ vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} } };
		if(mHasDepth)
			clearColors.emplace_back(vk::ClearDepthStencilValue{ 1.0f, 0 });

		vk::RenderPassBeginInfo beginInfo{};
		beginInfo.renderPass = mRenderPass;
		beginInfo.framebuffer = mFramebuffers[VkSurface::BoundSurface()->CurrentFrame()];
		beginInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
		beginInfo.renderArea.extent = vk::Extent2D{ mWidth, mHeight };
		beginInfo.clearValueCount = static_cast<u32>(clearColors.size());
		beginInfo.pClearValues = clearColors.data();

		vkCmd->CurrentBuffer().setViewport(0, { { 0.0f, 0.0f, static_cast<f32>(mWidth), static_cast<f32>(mHeight), 0.0f, 1.0f } });
		vkCmd->CurrentBuffer().setScissor(0, vk::Rect2D{ {{0, 0}, mWidth, mHeight } });

		vkCmd->CurrentBuffer().beginRenderPass(beginInfo, vk::SubpassContents::eInline);
	}

	void VkRenderPass::End(const std::shared_ptr<CommandBuffer>& cmd) const
	{
		const auto& vkCmd = std::static_pointer_cast<VkCommandBuffer>(cmd);

		vkCmd->CurrentBuffer().endRenderPass();
	}

	void VkRenderPass::CreateAttachments(const vk::Format format, const vk::ImageLayout layout)
	{//TODO: get formats from images
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

		if (!mHasDepth)
			return;

		vk::AttachmentDescription& depthAttachment{ mAttachments.emplace_back() };
		depthAttachment.format = vk::Format::eD32Sfloat;
		depthAttachment.samples = vk::SampleCountFlagBits::e1;
		depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
		depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::AttachmentReference& depthAttachmentRef{ mReferences.emplace_back() };
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	}

	void VkRenderPass::CreateSubpasses()
	{
		vk::SubpassDescription& subpass{ mSubpasses.emplace_back() };
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &mReferences[0];
		subpass.pDepthStencilAttachment = mHasDepth ? &mReferences[1] : nullptr;
	}

	void VkRenderPass::CreateDependencies(const vk::PipelineStageFlagBits pipelineStage, const vk::AccessFlags access)
	{
		const auto srcStage{ mHasDepth ? pipelineStage | vk::PipelineStageFlagBits::eEarlyFragmentTests : pipelineStage };
		const auto dstStage{ mHasDepth ? vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests : vk::PipelineStageFlagBits::eColorAttachmentOutput };
		const auto dstMask{ mHasDepth ? vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite : vk::AccessFlagBits::eColorAttachmentWrite };

		vk::SubpassDependency& dependency{ mDependencies.emplace_back() };
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcStageMask = srcStage;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = dstStage;
		dependency.dstAccessMask = dstMask;
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

	void VkRenderPass::CreateFramebuffer(const std::vector<vk::ImageView>& views, const u32 currentFrame)
	{
		vk::FramebufferCreateInfo createInfo{};
		createInfo.renderPass = mRenderPass;
		createInfo.attachmentCount = static_cast<u32>(views.size());
		createInfo.pAttachments = views.data();
		createInfo.width = mWidth;
		createInfo.height = mHeight;
		createInfo.layers = 1;

		mFramebuffers[currentFrame] = VkCore::Get()->Device().createFramebuffer(createInfo);
	}

	void VkRenderPass::CreateImage(const u32 currentFrame)
	{
		mImages[currentFrame] = std::make_unique<VkImage>(
			mWidth, 
			mHeight, 
			vk::Format::eB8G8R8A8Srgb, 
			vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment, 
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageAspectFlagBits::eColor);
	}

	void VkRenderPass::CreateDepthImage(const u32 currentFrame)
	{
		mDepthImages[currentFrame] = std::make_unique<VkImage>(
			mWidth,
			mHeight,
			vk::Format::eD32Sfloat,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::ImageLayout::eDepthStencilAttachmentOptimal,
			vk::ImageAspectFlagBits::eDepth);
	}
}
