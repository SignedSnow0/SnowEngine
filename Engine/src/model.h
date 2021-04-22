#pragma once
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "device.h"
#include "mesh.h"
#include "Buffers/texture.h"
#include "pipeline.h"

namespace SnowEngine {
    class Model {
    public:
        Model(Device& device, const std::string& modelPath);
        ~Model();

        inline VkDescriptorSet          GetDescriptorSet(uint32_t frame)        { return descriptorSets[frame]; }
        inline glm::mat4                GetPushConstant()                       { return pushConstant; }
        inline void                     SetPushConstant(glm::mat4 pushConstant) { this->pushConstant = pushConstant; }
        inline VkDescriptorSetLayout    GetDescriptorLayout()                   { return descriptorLayout; }
        inline void BindPipeline(Pipeline* pipeline) { this->pipeline = pipeline; }
        inline Pipeline* GetPipeline() { return pipeline; }

        void Draw(VkCommandBuffer commandBuffer, size_t imageIndex);
        
    private:
        void    CreateDescriptorSets();
        void    LoadModel(const std::string& path);
        void    ProcessNode(aiNode* node, const aiScene* scene);
        Mesh*   ProcessMesh(aiMesh* mesh, const aiScene* scene, aiNode* node);
        std::vector<Texture*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string typeName);

    private:
        Device& device;
        Pipeline* pipeline;

        std::vector<Mesh*>                  meshes;
        std::vector<VkDescriptorSet>        descriptorSets;
        VkDescriptorSetLayout               descriptorLayout;
        std::vector<Texture*>               texturesLoaded;
        glm::mat4                           pushConstant;
        aiMatrix4x4 currentTransform{};
        std::string path;
    };
}