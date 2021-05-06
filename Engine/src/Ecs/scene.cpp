#include "scene.h"

#include <imguiLib/imgui.h>
#include <imgui_internal.h>

#include "entity.h"
#include "generalComponents.h"
#include "imguiLib/imgui.h"
#include "application.h"
#include "lightComponents.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SnowEngine {
	static Entity selectedEntity;

	Scene::Scene() {
		Application& app = Application::Get();
		app.OnUpdate += std::bind(&Scene::Update, this, std::placeholders::_1, std::placeholders::_2);
	}

	Scene::~Scene() {

	}

	SnowEngine::Entity Scene::CreateEntity(const std::string& name) {
		Entity entity{ m_registry.create(), this };

		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
		return entity;
	}

	void Scene::DeleteEntity(Entity entity) {
		m_registry.destroy(entity);
	}

	bool Scene::Update(uint32_t frame, float deltaTime) {
		ImGuiDraw();

		{
			auto view = m_registry.view<PointLightComponent>();
			for (auto entity : view) {
				PointLight* light = view.get<PointLightComponent>(entity).Light;
				light->Update(frame);
			}
		}
		{
			auto view = m_registry.view<SpotLightComponent>();
			for (auto entity : view) {
				SpotLight* light = view.get<SpotLightComponent>(entity).Light;
				light->Update(frame);
			}
		}
		{
			auto view = m_registry.view<DirectionalLightComponent>();
			for (auto entity : view) {
				DirectionalLight* light = view.get<DirectionalLightComponent>(entity).Light;
				light->Update(frame);
			}
		}

		return true;
	}

	void Scene::ImGuiDraw() {
		ImGui::Begin("Scene");

		m_registry.each([&](auto entityID) {
			Entity entity{ entityID, this };
			DrawEntityNode(entity); //per ogni entità disegno il suo nodo nella lista di entità
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			selectedEntity = {}; //resets selection

		if (ImGui::BeginPopupContextWindow(0, 1, false)) {
			if (ImGui::MenuItem("Add Entity")) {
				CreateEntity("New Entity");
			}
			ImGui::EndPopup();
		}
		ImGui::End();

		ImGui::Begin("Components");
		if (selectedEntity) { //se un entità è selezionata ne disegno i componenti
			DrawComponents(selectedEntity);
		}
		ImGui::End();

		if(ImGui::Begin("Shadow Settings")) {
			Application& app = Application::Get();

			float bias = app.shadowMap->GetConstantBias();
			float slope = app.shadowMap->GetBiasSlope();

			ImGui::SliderFloat("Bias", &bias, 0.0f, 10.0f);
			ImGui::SliderFloat("Bias slope", &slope, 0.0f, 10.0f);

			app.shadowMap->SetConstantBias(bias);
			app.shadowMap->SetBiasSlope(slope);
		}
		ImGui::End();
	}

	void Scene::DrawEntityNode(Entity entity) {
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked()) {
			selectedEntity = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened) {
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			if (opened)
				ImGui::TreePop();
			ImGui::TreePop();
		}

		if (entityDeleted) {
			DeleteEntity(entity);
			if (selectedEntity == entity)
				selectedEntity = {};
		}
	}

	void Scene::DrawComponents(Entity entity) {
		if (entity.HasComponent<TagComponent>()) {
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, tag.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("Select a component");

		if (ImGui::BeginPopup("Select a component")) {
			DrawMenuItem<TransformComponent>("Transform");
			DrawMenuItem<ModelComponent>("Model");
			DrawMenuItem<ShadowCastComponent>("Shadow");
			ImGui::EndPopup();
		}
		
		if (entity.HasComponent<TransformComponent>()) {
			DrawComponent<TransformComponent>(entity, "Transform");
		}

		if (entity.HasComponent<ModelComponent>()) {
			DrawComponent<ModelComponent>(entity, "Model");
		}

		if (entity.HasComponent<SpotLightComponent>()) {
			DrawComponent<SpotLightComponent>(entity, "Spotlight");
		}

		if (entity.HasComponent<DirectionalLightComponent>()) {			
			DrawComponent<DirectionalLightComponent>(entity, "Directional light");
		}

		if (entity.HasComponent<PointLightComponent>()) {
			DrawComponent<PointLightComponent>(entity, "Point light");
		}

		if (entity.HasComponent<ShadowCastComponent>()) {
			DrawComponent<ShadowCastComponent>(entity, "Shadow Casting");
		}
	}

	template<typename T>
	void Scene::DrawMenuItem(const std::string label) {
		if (ImGui::MenuItem(label.c_str())) {
			if (!selectedEntity.HasComponent<T>())
				selectedEntity.AddComponent<T>();
			else
				std::cout << "This element already has this component!" << std::endl;
			ImGui::CloseCurrentPopup();
		}
	}

	template<typename T>
	void Scene::DrawComponent(Entity entity, std::string label) {
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		auto& component = entity.GetComponent<T>();
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImGui::Separator();
		bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, label.c_str());
		ImGui::PopStyleVar();

		ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
		if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight })) {
			ImGui::OpenPopup("Component options");
		}

		bool removeComponent = false;
		if (ImGui::BeginPopup("Component options")) {
			if (ImGui::MenuItem("Remove component"))
				removeComponent = true;

			ImGui::EndPopup();
		}

		if (open) {
			//component specific drawing
			component.ImGuiDraw();

			ImGui::TreePop();
		}

		if (removeComponent)
			entity.RemoveComponent<T>();
	}

	void Scene::Draw(uint32_t frame, VkCommandBuffer buffer) {	
		{
			Application& app = Application::Get();
			glm::mat4 viewMat = glm::lookAt(-app.dLight.GetDirection() * 200.0f, app.dLight.GetDirection(), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.001f, 1000.0f);
			proj[1][1] *= -1;
			
			app.shadowMap->BeginRenderPass(frame, buffer, proj * viewMat);

			auto view = m_registry.view<ModelComponent, TransformComponent, ShadowCastComponent>();
			for (auto entity : view) {
				glm::mat4 transform = view.get<TransformComponent>(entity);
				vkCmdPushConstants(buffer, app.shadowMap->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);

				ModelComponent mComponent = view.get<ModelComponent>(entity);
				ShadowCastComponent sComponent = view.get<ShadowCastComponent>(entity);
				if (sComponent)
					app.shadowMap->RenderShadowMap(buffer, mComponent);
			}
			app.shadowMap->EndRenderPass(frame, buffer);		
		}

		{
			Application::Get().BeginRenderPass(frame);

			Pipeline* pipeline = Application::Get().GetPipeline();
			vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());

			vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), 0, 1, &Application::Get().globalDescriptorSets[frame], 0, nullptr);//todo create descriptors in scene

			auto view = m_registry.view <ModelComponent, TransformComponent>(); //tutte le entità con un modelcomponent
			for (auto entity : view) {
				Model* model = view.get<ModelComponent>(entity).model; //ottengo il component di una specifica entità
				if (model != nullptr) {
					glm::mat4 transform = view.get<TransformComponent>(entity);
					vkCmdPushConstants(buffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);

					model->Draw(buffer, frame, pipeline->GetLayout());
				}
			}
		}
	}
}