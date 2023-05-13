#pragma once
#include <filesystem>

namespace SnowEngine
{
	enum class ShaderType
	{
		Vertex,
		Fragment,
		Compute
	};

	using shaderSource = std::tuple<std::filesystem::path, ShaderType>;

	struct GraphicShaderSource
	{
		shaderSource Vertex;
		shaderSource Fragment;

		std::vector<shaderSource> Others;
	};

	struct ComputeShaderSource
	{
		shaderSource Comp;
	};

	class Shader
	{
	public:
		static std::shared_ptr<Shader> Create(const GraphicShaderSource& source);
		static std::shared_ptr<Shader> Create(const ComputeShaderSource& source);
	};
}
