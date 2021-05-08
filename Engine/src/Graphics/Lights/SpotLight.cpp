#include "SpotLight.h"

namespace SnowEngine {
	SpotLight::SpotLight(Device& device) : device(device) {

	}

	SpotLight::~SpotLight() {
		vkQueueWaitIdle(device.GetGraphicsQueue());
	}

}