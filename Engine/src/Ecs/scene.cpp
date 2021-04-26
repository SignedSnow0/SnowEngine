#include "scene.h"

#include <imguiLib/imgui.h>
#include <imgui_internal.h>

#include "entity.h"
#include "generalComponents.h"
#include "imguiLib/imgui.h"
#include "application.h"

namespace SnowEngine {
	static Entity selectedEntity;

	Scene::Scene() {
		Application& app = Application::Get();
		app.OnUpdate += std::bind(&Scene::Update, this, std::placeholders::_1, std::placeholders::_2);
	}

	Scene::~Scene() {

	}

	SnowEngine::Entity& Scene::CreateEntity(const std::string& name) {
		Entity entity{ m_registry.create(), this };

		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
		entity.AddComponent<TransformComponent>();
		return entity;
	}

	void Scene::DeleteEntity(Entity entity) {
		m_registry.destroy(entity);
	}

	bool Scene::Update(uint32_t frame, float deltaTime) {
		if (ImGui::Begin("Scene")) {
			m_registry.each([&](entt::entity entityHandle) {
				Entity entity{ entityHandle, this };
				DrawEntityNode(entity); //per ogni entità disegno il suo nodo nella lista di entità
			});
		}
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

		return true;
	}

	void Scene::DrawEntityNode(Entity entity) {
		TagComponent& tag = entity.GetComponent<TagComponent>();
		ImGuiTreeNodeFlags flags = ((selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.Tag.c_str());
		if (ImGui::IsItemClicked())
			selectedEntity = entity;

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened) {
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.Tag.c_str());
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

			ImGui::EndPopup();
		}
		
		if (entity.HasComponent<TransformComponent>()) {
			DrawComponent<TransformComponent>(entity, "Transform");
		}

		if (entity.HasComponent<ModelComponent>()) {
			DrawComponent<ModelComponent>(entity, "Model");
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
		auto view = m_registry.view<ModelComponent, TransformComponent>(); //tutte le entità con un modelcomponent

		for (auto entity : view) {
			bool found = false;
			auto model = view.get<ModelComponent>(entity); //ottengo il component di una specifica entità
			if (model.model != nullptr) {
				for (Pipeline* pipeline : pipelines) {
					if (pipeline->GetConfig().layouts[1] == model.model->GetDescriptorLayout()) {
						vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());

						glm::mat4 transform = view.get<TransformComponent>(entity);
						vkCmdPushConstants(buffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);
						vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), 0, 1, &Application::Get().globalDescriptorSets[frame], 0, nullptr);//todo create descriptors in scene

						model.model->Draw(buffer, frame, pipeline->GetLayout());
						found = true;
					}
				}
				if (!found) {
					Pipeline::PipelineConfig pipConfig = Pipeline::FillPipelineConfig();
					pipConfig.renderPass = Application::Get().swapChain->GetRenderPass();
					pipConfig.pushConstant = Application::Get().pushConstant;
					pipConfig.layouts.insert({ 0, Application::Get().globalDescriptorLayout });
					pipConfig.layouts.insert({ 1, model.model->GetDescriptorLayout() });

					pipelines.push_back(new Pipeline(Device::Get(), pipConfig, "resources/shaders/spirv/base_shader.vert.spv", "resources/shaders/spirv/mapping_shader.frag.spv"));
				}
			}	
		}
	}
}