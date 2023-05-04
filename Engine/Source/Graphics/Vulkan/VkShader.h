#pragma once
#include <filesystem>
#include <map>
#include <vulkan/vulkan.hpp>
#include "Graphics/Rhi/Shader.h"
#include "Core/Types.h"

namespace SnowEngine
{
	using binding = u32;
	using set = u32;

	enum class VkResourceType : u32
	{
		Uniform,
		Image
	};

	struct VkResource
	{
		VkResourceType Type;
		std::string Name;
		u32 Size;
		vk::DescriptorSetLayoutBinding LayoutBinding;
	};

	struct VkDescriptorSetLayout
	{
		std::map<binding, VkResource> Resources;
		set SetIndex;

		::VkDescriptorSetLayout CreateLayout() const;
	};

	class VkShader : public Shader
	{
	public:
		VkShader(const GraphicShaderSource& source);

		std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageInfos() const;
		const std::map<set, VkDescriptorSetLayout>& GetLayouts() const;

	private:
		void CreateModule(const std::vector<u32>& spv, vk::ShaderStageFlagBits stage);
		void CreateReflection(const std::vector<u32>& spv, vk::ShaderStageFlagBits stage);

		static std::vector<u32> Compile(const shaderSource& source);

		using shaderInfo = std::tuple<vk::ShaderModule, vk::ShaderStageFlagBits>;
		std::vector<shaderInfo> mModules;
		std::map<set, VkDescriptorSetLayout> mDescriptorSetLayouts;
	};
}
