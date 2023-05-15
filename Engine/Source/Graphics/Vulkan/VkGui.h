#pragma once
#include <vulkan/vulkan.hpp>
#include <imgui.h>

#include "VkSurface.h"
#include "VkRenderPass.h"
#include "Graphics/Rhi/Gui.h"

namespace SnowEngine
{
	class VkGui : public Gui
	{
	public:
		VkGui(std::shared_ptr<const VkSurface> surface, std::shared_ptr<VkRenderPass> scene);
		~VkGui() override;

		void Begin(const std::shared_ptr<CommandBuffer>& cmd) override;
		void End(const std::shared_ptr<CommandBuffer>& cmd) override;

	private:
		void CreateDescriptorPool();
		void CreateSampler();
		void InitImGui() const;
		void CreateSceneImage(const VkImage& image, u32 frameIndex);

		vk::DescriptorPool mDescriptorPool;
		VkRenderPass mRenderPass;
		std::shared_ptr<const VkSurface> mSurface;
		std::shared_ptr<VkRenderPass> mScene;
		std::vector<ImTextureID> mSceneImages;
		vk::Sampler mSampler;
	};
}
