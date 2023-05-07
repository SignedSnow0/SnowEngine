#include "VkGui.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include "VkCore.h"

namespace SnowEngine
{
	VkGui::VkGui(std::shared_ptr<const VkSurface> surface, std::shared_ptr<VkRenderPass> scene)
		: mRenderPass{ surface }, mSurface{ std::move(surface) }, mScene{ std::move(scene) }
	{
		CreateDescriptorPool();
		InitImGui();
		CreateSampler();
		CreateSceneImages();
	}

	VkGui::~VkGui()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VkGui::Begin() const
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();//TODO: remove

		ImGui::DockSpaceOverViewport();

		mRenderPass.Begin();
	}

	void VkGui::End()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		if (ImGui::Begin("Scene"))
		{
			ImGui::Image(mSceneImages[mSurface->GetCurrentFrame()], ImGui::GetContentRegionAvail());
		}
		ImGui::PopStyleVar(1);

		ImGui::End();

		ImGui::Render();

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VkSurface::BoundSurface()->GetCommandBuffer());
		mRenderPass.End();
	}

	void VkGui::CreateDescriptorPool()
	{
		const std::vector<vk::DescriptorPoolSize> poolSizes
		{
			{ vk::DescriptorType::eSampler, 1000 },
			{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			{ vk::DescriptorType::eSampledImage, 1000 },
			{ vk::DescriptorType::eStorageBuffer, 1000 },
			{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
			{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
			{ vk::DescriptorType::eUniformBuffer, 1000 },
			{ vk::DescriptorType::eStorageBuffer, 1000 },
			{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
			{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
			{ vk::DescriptorType::eInputAttachment, 1000 },
		};

		vk::DescriptorPoolCreateInfo createInfo{};
		createInfo.maxSets = 1000;
		createInfo.poolSizeCount = static_cast<u32>(poolSizes.size());
		createInfo.pPoolSizes = poolSizes.data();
		createInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

		mDescriptorPool = VkCore::Get()->Device().createDescriptorPool(createInfo);
	}

	void VkGui::CreateSampler()
	{
		vk::SamplerCreateInfo createInfo{};
		createInfo.magFilter = vk::Filter::eLinear;
		createInfo.minFilter = vk::Filter::eLinear;
		createInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		createInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		createInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		createInfo.mipLodBias = 0.0f;
		createInfo.anisotropyEnable = VK_FALSE;
		createInfo.maxAnisotropy = 1.0f;
		createInfo.compareEnable = VK_FALSE;
		createInfo.compareOp = vk::CompareOp::eAlways;
		createInfo.minLod = 0.0f;
		createInfo.maxLod = 1.0f;
		createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;

		mSampler = VkCore::Get()->Device().createSampler(createInfo);
	}

	void VkGui::InitImGui()
	{
		ImGui::CreateContext();
		ImGuiIO& io{ ImGui::GetIO() };
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplGlfw_InitForVulkan(mSurface->GetWindow()->Handle(), true);

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = VkCore::Get()->Instance();
		initInfo.PhysicalDevice = VkCore::Get()->PhysicalDevice();
		initInfo.Device = VkCore::Get()->Device();
		initInfo.Queue = VkCore::Get()->Queues()[0].Queue;
		initInfo.QueueFamily = VkCore::Get()->Queues()[0].Family;
		initInfo.DescriptorPool = mDescriptorPool;
		initInfo.MinImageCount = 2;
		initInfo.ImageCount = 2; //TODO: surface
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&initInfo, mRenderPass.RenderPass());

		VkCore::Get()->SubmitInstantCommand([](const vk::CommandBuffer cmd)
		{
			ImGui_ImplVulkan_CreateFontsTexture(cmd);
		});

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void VkGui::CreateSceneImages()
	{
		mSceneImages.clear();
		mSceneImages.reserve(mScene->Images().size());
		for (const auto& image : mScene->Images())
			mSceneImages.emplace_back(ImGui_ImplVulkan_AddTexture(mSampler, image->GetView(), static_cast<VkImageLayout>(image->GetLayout())));
	}
}
