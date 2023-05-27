#pragma once

namespace SnowEngine
{
	class GraphicsCore
	{
	public:
		static GraphicsCore* Init();
		static void Shutdown();

		virtual ~GraphicsCore() = default;

		static void WaitIdle();

	protected:
		GraphicsCore() = default;

		virtual void DeviceWaitIdle() const = 0;

	private:
		static GraphicsCore* sInstance;
	};
}