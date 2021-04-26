#pragma once
#include "Event.hpp"
#include "Ecs/entity.h"
#include "Ecs/generalComponents.h"

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