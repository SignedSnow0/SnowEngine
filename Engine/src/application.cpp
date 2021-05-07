#include "application.h"
#include <stdexcept>
#include <chrono>
#include <filesystem>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imguiLib/imgui.h>
#include <nfd.h>

#include "input/mouse.h"
#include "Ecs/entity.h"
#include "Ecs/generalComponents.h"
#include "Ecs/lightComponents.h"

namespace SnowEngine {
    Application* Application::app = nullptr;

    Application::Application() {
        app = this;
        scene = new Scene();
        OnResize();
        camera = new Camera(device, glm::vec3(1.0f));
        shadowMap = new ShadowMap(device, 5, 6, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        CreateGloalDescriptorSets();

        VkPushConstantRange pushConstant{};
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstant.offset = 0;
        pushConstant.size = sizeof(glm::mat4);

        Pipeline::PipelineConfig config{};
        config = Pipeline::FillPipelineConfig();      
        config.renderPass = swapChain->GetRenderPass();
        config.pushConstant = &pushConstant;
        config.layouts.insert({ 0, globalDescriptorLayout });
        config.layouts.insert({ 1, startingEntity.GetDescriptorLayout() });
        pipeline = new Pipeline(device, config, "resources/shaders/spirv/shader.vert.spv", "resources/shaders/spirv/shader.frag.spv");

        CreateCommandBuffers();
        imguiLayer = new ImGuiLayer(window, device, *swapChain.get());

        OnUpdate += std::bind(&Application::Update, this, std::placeholders::_1, std::placeholders::_2);

        entities.push_back(scene->CreateEntity("Starting entity"));
        entities[0].AddComponent<ModelComponent>(&startingEntity);
        entities[0].AddComponent<TransformComponent>();

        entities.push_back(scene->CreateEntity("Directional light"));
        entities[1].AddComponent<DirectionalLightComponent>(&dLight);

        entities.push_back(scene->CreateEntity("Point light"));
        entities[2].AddComponent<PointLightComponent>(&pLight);

        entities.push_back(scene->CreateEntity("Spot light"));
        entities[3].AddComponent<SpotLightComponent>(&sLight);

        skybox = new Skybox(device, textures, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    Application::~Application() {
        delete imguiLayer;
        delete scene;
        delete pipeline;

        vkDestroyDescriptorSetLayout(device, globalDescriptorLayout, nullptr);
        vkFreeDescriptorSets(device, device.GetDescriptorPool(), globalDescriptorSets.size(), globalDescriptorSets.data());

        glfwTerminate();
    }

    void Application::Run() {
        float deltaTime = 0.0f;	// Time between current frame and last frame
        float lastFrame = 0.0f;

        while (!window.ShouldClose()) {
            uint32_t index = GetFrame();
            if (index >= 0) {
                float currentFrame = glfwGetTime();
                deltaTime = currentFrame - lastFrame;
                lastFrame = currentFrame;

                imguiLayer->BeginFrame();
                OnUpdate.Dispatch(index, deltaTime);
                Update(index, deltaTime);
                Draw(index);
            }

            glfwPollEvents();
        }
        vkDeviceWaitIdle(device); //prima di chiudere il programma aspettiamo che tutti i cicli siano conclusi per evitare errori
    }

    uint32_t Application::GetFrame() {
        uint32_t index;
        VkResult result = swapChain->AcquireImage(&index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            OnResize();
            return -1;
        }

        return index;
    }

    bool Application::Update(uint32_t frame, float deltaTime) {

        return true;
    }

    void Application::CreateGloalDescriptorSets() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = { camera->GetLayoutBindings() };
		bindings.push_back(dLight.GetBuffer()->GetLayoutBinding());
		bindings.push_back(sLight.GetBuffer()->GetLayoutBinding());
		bindings.push_back(pLight.GetBuffer()->GetLayoutBinding());
        auto shadowBindings = shadowMap->GetLayoutBindings();
        bindings.insert(bindings.end(), shadowBindings.begin(), shadowBindings.end());

        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        createInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &globalDescriptorLayout))
            throw std::runtime_error("Failed to create descriptor set layout!");

        std::vector<VkDescriptorSetLayout> layouts(3, globalDescriptorLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = device.GetDescriptorPool();
        allocInfo.descriptorSetCount = static_cast<uint32_t>(3);
        allocInfo.pSetLayouts = layouts.data();

        globalDescriptorSets.resize(3);
        if (vkAllocateDescriptorSets(device, &allocInfo, globalDescriptorSets.data()) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate descriptor sets!");

        for (size_t i = 0; i < 3; i++) {
            std::vector<VkWriteDescriptorSet> descriptorWrites{ camera->GetDescriptorWrites(i, globalDescriptorSets[i]) };
			descriptorWrites.push_back(dLight.GetBuffer()->CreateDescriptorWrite(i, globalDescriptorSets[i]));
			descriptorWrites.push_back(sLight.GetBuffer()->CreateDescriptorWrite(i, globalDescriptorSets[i]));
			descriptorWrites.push_back(pLight.GetBuffer()->CreateDescriptorWrite(i, globalDescriptorSets[i]));
            auto shadowWrites = shadowMap->GetDescriptorWrites(i, globalDescriptorSets[i]);
            descriptorWrites.insert(descriptorWrites.end(), shadowWrites.begin(), shadowWrites.end());

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void Application::Draw(uint32_t frame) {
        BeginCommandBuffer(frame);

        RecordCommandBuffer(frame);
        imguiLayer->EndFrame(commandBuffers[frame]);

        EndRenderPass(frame);
        EndCommandBuffer(frame);

        VkResult result = swapChain->SubmitCommandBuffer(&commandBuffers[frame], &frame);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.HasResized()) {
            auto extent = window.GetExtent();
            camera->SetWindowSize({ extent.width, extent.height });
            OnResize();
            return;
        }
    }

    void Application::CreateCommandBuffers() {
        commandBuffers.resize(swapChain->GetImageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool           = device.GetCommandPool();
        allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount    = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate command buffers!");
    }

    void Application::BeginCommandBuffer(uint32_t i) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags             = 0; // Optional
        beginInfo.pInheritanceInfo  = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to begin recording command buffer!");      
    }

	void Application::BeginRenderPass(uint32_t i) {
		std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = { 0.49f, 0.25f, 0.0f, 1.0f }; //color buffer
		clearValues[1].depthStencil = { 1.0f, 0 }; //depth buffer

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapChain->GetRenderPass();
		renderPassInfo.framebuffer = swapChain->GetFrameBuffer(i);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->GetExtent();
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain->GetExtent().width);
		viewport.height = static_cast<float>(swapChain->GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0,0 };
		scissor.extent = swapChain->GetExtent();
		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);
	}

	void Application::EndCommandBuffer(uint32_t i) {
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffer!");
    }

	void Application::EndRenderPass(uint32_t i) {
        vkCmdEndRenderPass(commandBuffers[i]);
	}

	void Application::RecordCommandBuffer(uint32_t i) {
        scene->Draw(i, commandBuffers[i]);
    }

    void Application::OnResize() {
        auto extent = window.GetExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = window.GetExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(device);

        swapChain.reset();
        swapChain = std::make_unique<SwapChain>(device, extent);
    }
}