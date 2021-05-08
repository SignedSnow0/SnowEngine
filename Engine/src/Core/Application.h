#pragma once
#include <memory>

#include "Core/ImguiLib/ImguiLayer.h"
#include "Core/Window.h"
#include "Vulkan/Device.h"
#include "Vulkan/SwapChain.h"
#include "Events/OtherEvents.hpp"
#include "Ecs/Scene.h"
#include "Graphics/Lights/DirectionalLight.h"
#include "Graphics/Lights/SpotLight.h"
#include "Graphics/Lights/PointLight.h"
#include "Graphics/ShadowMapping/ShadowMap.h"
#include "Graphics/Skybox.h"
#include "Graphics/Camera/camera.h"

namespace SnowEngine {
    class Application {
    public:
        Application();
        ~Application();

        inline static Application& Get() { return *app; }
        inline bool ImguiEnabled() { return imguiLayer->IsEnabled(); }
        inline SwapChain& GetSwapchain() { return *swapChain; }
        //Starts the Update Render cycle
        void Run();
    public:
        UpdateEvent OnUpdate{};

    private:
        //
        void                BeginCommandBuffer(uint32_t i);
        //
        void                BeginRenderPass(uint32_t i);
        //
        void                EndCommandBuffer(uint32_t i);
        //
        void                EndRenderPass(uint32_t i);
        //
        void                CreateCommandBuffers();
        //
        void                Draw(uint32_t frame);
        //
        uint32_t            GetFrame();
        //
        void                OnResize();
        //
        void                RecordCommandBuffer(uint32_t i);
        //
        bool                Update(uint32_t frame, float deltaTime);
        
        void CreateGloalDescriptorSets();
        Pipeline* GetPipeline() { return pipeline; }

    private:
        static Application* app;
        
        Window window{ 1920, 1080, "SnowEngine" };
        Device device{ window };
        std::unique_ptr<SwapChain> swapChain;
        std::vector<VkCommandBuffer> commandBuffers;
        ImGuiLayer* imguiLayer;
        Pipeline* pipeline;
        Camera* camera;
        Model startingEntity{ device, "resources/models/sphere.obj" };
		
        std::vector<VkDescriptorSet> globalDescriptorSets;
        VkDescriptorSetLayout globalDescriptorLayout;
        Scene* scene; 

		DirectionalLight dLight{ device };
		SpotLight sLight{ device };
		PointLight pLight{ device };
		PointLight pLight2{ device };
        std::vector<Entity> entities;

        ShadowMap* shadowMap;

        std::vector<std::string> textures = { "resources/textures/skybox/right.jpg", "resources/textures/skybox/left.jpg", "resources/textures/skybox/top.jpg", "resources/textures/skybox/bottom.jpg", "resources/textures/skybox/front.jpg", "resources/textures/skybox/back.jpg" };
        Skybox* skybox;

        friend class Scene;
    };
}