#pragma once
#include "Event.hpp"
#include "Ecs/Entity.h"
#include "Ecs/GeneralComponents.h"

namespace SnowEngine {
	class AddComponentEvent : public Event<Entity, Component> {
		AddComponentEvent() : Event(EventType::Component) {}

		void Dispatch(Entity sender, Component component) override {
			if (component.Type == "ModelComponent") {

			}

			if (showToConsole)
				std::cout << "[Add component event]: Component: " << component.Type << std::endl;
		}
	};
}