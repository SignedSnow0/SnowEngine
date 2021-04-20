#include "application.h"
#include <stdexcept>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imguiLib/imgui.h>

#include "input/mouse.h"

namespace SnowEngine {
    Application* Application::app = nullptr;

    Application::Application() {
        app = this;
        OnResize();

        VkPushConstantRange pushConstant;
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstant.offset = 0;
        pushConstant.size = sizeof(glm::mat4);

        Pipeline::PipelineConfig config{};
        config = Pipeline::FillPipelineConfig();      
        config.renderPass = swapChain->GetRenderPass();
        config.pushConstant = pushConstant;
        config.layouts.insert({ 1, backPack.GetDescriptorLayout() });
        light = new Light(device);
        camera = new Camera(device, glm::vec3(1.0f), config, light);

        config.rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
        cameraWire = new Camera(device, glm::vec3(1.0f), config, light);
        CreateCommandBuffers();

        imguiLayer = new ImGuiLayer(window, device, *swapChain.get());
    }

    Application::~Application() {
        delete light;

        delete camera;
        delete cameraWire;

        delete imguiLayer;

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
                Update(index);
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

    void Application::Update(uint32_t frame) {
        glm::mat4 transform;
        transform = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        transform *= glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
        backPack.SetPushConstant(transform);

        light->Update(frame);
    }

    void Application::Draw(uint32_t frame) {
        BeginCommandBuffer(frame);
        RecordCommandBuffer(frame);
        imguiLayer->EndFrame(commandBuffers[frame]);
        EndCommandBuffer(frame);

        VkResult result = swapChain->SubmitCommandBuffer(&commandBuffers[frame], &frame);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.HasResized()) {
            auto extent = window.GetExtent();
            camera->SetWindowSize({ extent.width, extent.height });
            cameraWire->SetWindowSize({ extent.width, extent.height });
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
        clearValues[0].color        = {0.0f, 0.0f, 0.0f, 1.0f}; //color buffer
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
        if (imguiLayer->IsEnabled()) {
            cameraWire->BindModel(&backPack);
            cameraWire->Draw(commandBuffers[i], i);
        }
        else {
            camera->BindModel(&backPack);
            camera->Draw(commandBuffers[i], i);
        }     
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