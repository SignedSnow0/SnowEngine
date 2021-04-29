#pragma once
#include "Lights/directionalLight.h"
#include "Lights/pointLight.h"
#include "Lights/spotLight.h"
#include "generalComponents.h"

namespace SnowEngine {
	struct DirectionalLightComponent : Component {
		DirectionalLight* Light;

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
		DirectionalLightComponent(DirectionalLight* light) : Light(light) {}

		void ImGuiDraw();
	};

	struct PointLightComponent : Component {
		PointLight* Light;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
		PointLightComponent(PointLight* light) : Light(light) {}

		void ImGuiDraw();
	};

	struct SpotLightComponent : Component {
		SpotLight* Light;

		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent&) = default;
		SpotLightComponent(SpotLight* light) : Light(light) {}

		void ImGuiDraw();
	};
}