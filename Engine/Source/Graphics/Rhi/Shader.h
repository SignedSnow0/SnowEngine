#pragma once
#include <filesystem>

namespace SnowEngine
{
	enum class ShaderType
	{
		Vertex,
		Fragment,
	};

	using shaderSource = std::tuple<std::filesystem::path, ShaderType>;

	struct GraphicShaderSource
	{
		shaderSource Vertex;
		shaderSource Fragment;

		std::vector<shaderSource> Others;
	};

	class Shader
	{
	public:
		static std::shared_ptr<Shader> Create(const GraphicShaderSource& source);
	};
}
