#pragma once
#include "Graphics/Lights/DirectionalLight.h"
#include "Graphics/Lights/PointLight.h"
#include "Graphics/Lights/SpotLight.h"
#include "GeneralComponents.h"

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

	struct ShadowCastComponent : Component {
		bool CastsShadow = true;

		ShadowCastComponent() = default;
		ShadowCastComponent(const ShadowCastComponent&) = default;
		ShadowCastComponent(const bool& castsShadow) : CastsShadow(castsShadow) {}

		operator const bool() { return CastsShadow; }

		void ImGuiDraw();
	};
}