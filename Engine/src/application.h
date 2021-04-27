#pragma once
#include <memory>

#include "window.h"
#include "device.h"
#include "swapChain.h"
#include "Camera/camera.h"
#include "Events/otherEvents.hpp"
#include "imguiLayer.h"
#include "Ecs/scene.h"

namespace SnowEngine {
    
    class Application {
    public:
        Application();
        ~Application();

        inline static Application& Get() { return *app; }
        inline bool ImguiEnabled() { return imguiLayer->IsEnabled(); }
        //Starts the Update Render cycle
        void Run();
    public:
        UpdateEvent OnUpdate{};

    private:
        //
        void                BeginCommandBuffer(uint32_t i);
        //
        void                EndCommandBuffer(uint32_t i);
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
        Light light{ device };
        Pipeline* pipeline;
        Camera* camera;
        Model startingEntity{ device, "resources/models/sphere.obj" };

        std::vector<VkDescriptorSet> globalDescriptorSets;
        VkDescriptorSetLayout globalDescriptorLayout;
        Scene* scene;
        VkPushConstantRange pushConstant;

        friend class Scene;
    };
}