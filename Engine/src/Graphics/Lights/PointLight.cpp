#include "PointLight.h"

namespace SnowEngine {
	StorageBuffer<PointLight::PointLightUBO>* PointLight::uBuffer;

	PointLight::PointLight(Device& device) : device(device) {
		if(uBuffer == nullptr)
			uBuffer = new StorageBuffer<PointLight::PointLightUBO>(Device::Get(), VK_SHADER_STAGE_FRAGMENT_BIT, 3, 3);
	}
}