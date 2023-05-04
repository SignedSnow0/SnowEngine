#pragma once

namespace SnowEngine
{
	class GraphicsCore
	{
	public:
		static GraphicsCore* Init();
		static void Shutdown();

		virtual ~GraphicsCore() = default;

	protected:
		GraphicsCore() = default;

	private:
		static GraphicsCore* sInstance;
	};
}