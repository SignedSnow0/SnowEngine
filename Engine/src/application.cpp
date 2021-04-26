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

namespace SnowEngine {
    Application* Application::app = nullptr;

    Application::Application() {
        app = this;
        scene = new Scene();
        OnResize();
        camera = new Camera(device, glm::vec3(1.0f));
        CreateGloalDescriptorSets();

        entities.push_back(new Model(device, "resources/models/backpack/backpack.gltf"));

        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstant.offset = 0;
        pushConstant.size = sizeof(glm::mat4);

        Pipeline::PipelineConfig config{};
        config = Pipeline::FillPipelineConfig();      
        config.renderPass = swapChain->GetRenderPass();
        config.pushConstant = pushConstant;
        config.layouts.insert({ 0, globalDescriptorLayout });
        config.layouts.insert({ 1, entities[0]->GetDescriptorLayout() });
        mappingPipeline = new Pipeline(device, config, "resources/shaders/spirv/base_shader.vert.spv", "resources/shaders/spirv/mapping_shader.frag.spv");
        //entities[0]->BindPipeline(mappingPipeline);

        config.layouts[1] = light.GetModel()->GetDescriptorLayout();
        basePipeline = new Pipeline(device, config, "resources/shaders/spirv/base_shader.vert.spv", "resources/shaders/spirv/base_shader.frag.spv");
        //light.GetModel()->BindPipeline(basePipeline);     

        CreateCommandBuffers();
        imguiLayer = new ImGuiLayer(window, device, *swapChain.get());

        OnUpdate += std::bind(&Application::Update, this, std::placeholders::_1, std::placeholders::_2);

        Entity entity = scene->CreateEntity("Backpack");
        entity.AddComponent<ModelComponent>(entities[0]);
    }

    Application::~Application() {
        delete imguiLayer;

        delete basePipeline;
        delete mappingPipeline;

        for (auto model : entities)
            delete model;

        vkDestroyDescriptorSetLayout(device, globalDescriptorLayout, nullptr);

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
        light.Update(frame, camera->GetPos());

        if (ImGui::Begin("Models")) {
            if (ImGui::Button("Load Model")) { //Premuto sul bottone
                nfdpathset_t resPath;
                nfdresult_t result = NFD_OpenDialogMultiple("obj,gltf", nullptr, &resPath);          
                if (result == NFD_OKAY) {
                    for (size_t i = 0; i < NFD_PathSet_GetCount(&resPath); ++i) {
                        nfdchar_t* path = NFD_PathSet_GetPath(&resPath, i);
                        Model* tmp = new Model(device, path);
                        //tmp->BindPipeline(basePipeline);
                        entities.push_back(tmp);
                    }
                    NFD_PathSet_Free(&resPath);
                }
            }     
        }
        ImGui::End();

        return true;
    }

    void Application::CreateGloalDescriptorSets() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = { camera->GetLayoutBinding(), light.GetLayoutBinding() };

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
            std::vector<VkWriteDescriptorSet> descriptorWrites;
            descriptorWrites.push_back(camera->GetDescriptorWrite(i, globalDescriptorSets[i]));
            descriptorWrites.push_back(light.GetDescriptorWrite(i, globalDescriptorSets[i]));

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void Application::Draw(uint32_t frame) {
        BeginCommandBuffer(frame);
        //RecordCommandBuffer(frame);
        scene->Draw(frame, commandBuffers[frame]);
        imguiLayer->EndFrame(commandBuffers[frame]);
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

        std::array<VkClearValue, 2> clearValues;
        clearValues[0].color        = {0.49f, 0.25f, 0.0f, 1.0f}; //color buffer
        clearValues[1].depthStencil = { 1.0f, 0 }; //depth buffer
        
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType                = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass           = swapChain->GetRenderPass();
        renderPassInfo.framebuffer          = swapChain->GetFrameBuffer(i);
        renderPassInfo.renderArea.offset    = {0, 0};
        renderPassInfo.renderArea.extent    = swapChain->GetExtent();
        renderPassInfo.clearValueCount      = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues         = clearValues.data();

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
        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffer!");
    }

    void Application::RecordCommandBuffer(uint32_t i) {
        //for (Model* entity : entities) {
            //camera->BindModel(entity);
        //}
        //camera->BindModel(light.GetModel());
        //camera->Draw(commandBuffers[i], i, globalDescriptorSets[i]);     
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