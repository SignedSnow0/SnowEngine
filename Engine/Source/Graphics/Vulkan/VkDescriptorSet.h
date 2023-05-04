#pragma once
#include <vulkan/vulkan.hpp>

#include "VkBuffers.h"
#include "VkShader.h"
#include "Graphics/Rhi/DescriptorSet.h"
#include "Graphics/Rhi/Image.h"

namespace SnowEngine
{
	class VkDescriptorSet : public DescriptorSet
	{
	public:
		VkDescriptorSet(const VkDescriptorSetLayout& layout, u32 frameCount);

		const std::vector<vk::DescriptorSet>& GetSets() const;
		set GetSetIndex() const;

		void SetUniform(const std::string& name, const void* data, u32 currentFrame) const override;
		void SetImage(const std::string& name, const Image* image) const override;

	private:
		void CreatePool();
		void CreateSets();
		void CreateBuffers();

		vk::DescriptorPool mPool;
		std::vector<vk::DescriptorSet> mSets;
		std::map<binding, std::unique_ptr<VkUniformBuffer>> mUniforms;
		std::map<binding, vk::Sampler> mImages;
		const VkDescriptorSetLayout& mLayout;
		u32 mFrameCount;
	};
}