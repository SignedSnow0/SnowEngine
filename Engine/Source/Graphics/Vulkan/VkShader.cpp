#include "VkShader.h"
#include <fstream>
#include <iostream>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "VkCore.h"

#ifdef _DEBUG
#pragma comment(lib, "shaderc_combinedd.lib")
#pragma comment(lib, "spirv-cross-cored.lib")
#pragma comment(lib, "spirv-cross-glsld.lib")
#else
#pragma comment(lib, "shaderc_combined.lib")
#pragma comment(lib, "spirv-cross-core.lib")
#pragma comment(lib, "spirv-cross-glsl.lib")
#endif

namespace SnowEngine
{
	static std::string ReadFile(const std::filesystem::path& path)
	{
		std::ifstream file{ path, std::ios::ate | std::ios::binary };
		if (!file.is_open())
			return "";

		const u64 fileSize{ static_cast<u64>(file.tellg()) };
		std::string buffer(fileSize, ' ');

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	static shaderc_shader_kind GetShaderKind(const ShaderType type)
	{
		switch (type)
		{
			case ShaderType::Vertex: return shaderc_vertex_shader;
			case ShaderType::Fragment: return shaderc_fragment_shader;
			case ShaderType::Compute: return shaderc_compute_shader;
			default: return shaderc_glsl_infer_from_source;
		}
	}

	static vk::ShaderStageFlagBits GetShaderStage(const ShaderType type)
	{
		switch (type)
		{
			case ShaderType::Vertex: return vk::ShaderStageFlagBits::eVertex;
			case ShaderType::Fragment: return vk::ShaderStageFlagBits::eFragment;
			case ShaderType::Compute: return vk::ShaderStageFlagBits::eCompute;
			default: return vk::ShaderStageFlagBits::eAll;
		}
	}

	::VkDescriptorSetLayout VkDescriptorSetLayout::CreateLayout() const
	{
		std::vector<vk::DescriptorSetLayoutBinding> bindings{};
		bindings.reserve(Resources.size());
		for (const auto& [binding, resource] : Resources)
			bindings.push_back(resource.LayoutBinding);

		vk::DescriptorSetLayoutCreateInfo createInfo{};
		createInfo.bindingCount = static_cast<u32>(bindings.size());
		createInfo.pBindings = bindings.data();

		return VkCore::Get()->Device().createDescriptorSetLayout(createInfo);
	}

	VkShader::VkShader(const GraphicShaderSource& source)
	{
		const auto& [vertex, fragment, others] = source;

		{
			const std::vector<u32> spv{ Compile(vertex) };
			CreateModule(spv, GetShaderStage(std::get<1>(vertex)));
			CreateReflection(spv, GetShaderStage(std::get<1>(vertex)));
		}
		{
			const std::vector<u32> spv{ Compile(fragment) };
			CreateModule(spv, GetShaderStage(std::get<1>(fragment)));
			CreateReflection(spv, GetShaderStage(std::get<1>(fragment)));
		}

		for (const auto& other : others)
		{
			const std::vector<u32> spv{ Compile(other) };
			CreateModule(spv, GetShaderStage(std::get<1>(other)));
			CreateReflection(spv, GetShaderStage(std::get<1>(other)));
		}
	}

	VkShader::VkShader(const ComputeShaderSource& source)
	{
		const std::vector<u32> spv{ Compile(source.Comp) };
		CreateModule(spv, vk::ShaderStageFlagBits::eCompute);
		CreateReflection(spv, vk::ShaderStageFlagBits::eCompute);
	}

	std::vector<vk::PipelineShaderStageCreateInfo> VkShader::ShaderStageInfos() const
	{
		std::vector<vk::PipelineShaderStageCreateInfo> infos;
		infos.resize(mModules.size());
		u32 i{ 0 };

		for (const auto& [module, stage] : mModules)
		{
			vk::PipelineShaderStageCreateInfo info{};
			info.stage = stage;
			info.module = module;
			info.pName = "main";

			infos[i] = info;
			i++;
		}

		return infos;
	}

	const std::map<set, VkDescriptorSetLayout>& VkShader::Layouts() const { return mDescriptorSetLayouts; }

	void VkShader::CreateModule(const std::vector<u32>& spv, const vk::ShaderStageFlagBits stage)
	{
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.codeSize = spv.size() * sizeof(u32);
		createInfo.pCode = spv.data();

		vk::ShaderModule module{ VkCore::Get()->Device().createShaderModule(createInfo) };

		mModules.emplace_back(module, stage);
	}

	void VkShader::CreateReflection(const std::vector<u32>& spv, const vk::ShaderStageFlagBits stage)
	{
		const spirv_cross::Compiler compiler{ spv };
		spirv_cross::ShaderResources resources{ compiler.get_shader_resources() };

		for (const auto& resource : resources.uniform_buffers)
		{
			const binding binding{ compiler.get_decoration(resource.id, spv::Decoration::DecorationBinding) };
			const set set{ compiler.get_decoration(resource.id, spv::Decoration::DecorationDescriptorSet) };

			const spirv_cross::SPIRType& type{ compiler.get_type(resource.base_type_id) };
			const u32 size{ static_cast<u32>(compiler.get_declared_struct_size(type)) };

			vk::DescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = binding;
			layoutBinding.descriptorCount = 1;
			layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
			layoutBinding.stageFlags = stage;

			VkResource res;
			res.LayoutBinding = layoutBinding;
			res.Name = resource.name;
			res.Size = size;
			res.Type = VkResourceType::Uniform;

			if (!mDescriptorSetLayouts.contains(set))
			{
				mDescriptorSetLayouts.insert({ set, {} });
				mDescriptorSetLayouts.at(set).SetIndex = set;
			}

			if (!mDescriptorSetLayouts.at(set).Resources.contains(binding))
				mDescriptorSetLayouts.at(set).Resources.insert({ binding, res });
		}

		for (const auto& resource : resources.sampled_images)
		{
			const binding binding{ compiler.get_decoration(resource.id, spv::Decoration::DecorationBinding) };
			const set set{ compiler.get_decoration(resource.id, spv::Decoration::DecorationDescriptorSet) };

			vk::DescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = binding;
			layoutBinding.descriptorCount = 1;
			layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding.stageFlags = stage;

			VkResource res;
			res.LayoutBinding = layoutBinding;
			res.Name = resource.name;
			res.Type = VkResourceType::Image;

			if (!mDescriptorSetLayouts.contains(set))
			{
				mDescriptorSetLayouts.insert({ set, {} });
				mDescriptorSetLayouts.at(set).SetIndex = set;
			}

			if (!mDescriptorSetLayouts.at(set).Resources.contains(binding))
				mDescriptorSetLayouts.at(set).Resources.insert({ binding, res });
		}

		for (const auto& resource : resources.storage_buffers)
		{
			const binding binding{ compiler.get_decoration(resource.id, spv::DecorationBinding) };
			const set set{ compiler.get_decoration(resource.id, spv::DecorationDescriptorSet) };

			const spirv_cross::SPIRType& type{ compiler.get_type(resource.base_type_id) };
			const u32 size{ static_cast<u32>(compiler.get_declared_struct_size(type)) };

			vk::DescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = binding;
			layoutBinding.descriptorCount = 1;
			layoutBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
			layoutBinding.stageFlags = stage;

			VkResource res;
			res.LayoutBinding = layoutBinding;
			res.Name = resource.name;
			res.Size = size;
			res.Type = VkResourceType::StorageBuffer;

			if (!mDescriptorSetLayouts.contains(set))
			{
				mDescriptorSetLayouts.insert({ set, {} });
				mDescriptorSetLayouts.at(set).SetIndex = set;
			}

			if (!mDescriptorSetLayouts.at(set).Resources.contains(binding))
				mDescriptorSetLayouts.at(set).Resources.insert({ binding, res });
		}
	}

	std::vector<u32> VkShader::Compile(const shaderSource& source)
	{
		const auto& [path, type] = source;
		const std::string code{ ReadFile(path) };

		const shaderc::Compiler compiler;
		const shaderc::SpvCompilationResult result{ compiler.CompileGlslToSpv(code, GetShaderKind(type), path.filename().string().c_str()) };
		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			std::cerr << result.GetErrorMessage() << std::endl;
			return {};
		}

		return { result.cbegin(), result.cend() };
	}
}
