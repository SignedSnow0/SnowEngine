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
        void                            Update();
        inline VkDescriptorSetLayout    GetDescriptorLayout()                   { return descriptorLayout; }

        inline glm::vec3    GetScale()                              { return scale; }
        inline glm::vec3    GetRotation()                           { return rotation; }
        inline glm::vec3    GetTranslation()                        { return translation; }
        inline void         SetScale(glm::vec3 scale)               { this->scale = scale; }
        inline void         SetRotation(glm::vec3 rotation)         { this->rotation = rotation; }
        inline void         SetTranslation(glm::vec3 translation)   { this->translation = translation; }

        inline std::string GetName() { return name; }

        void Draw(VkCommandBuffer commandBuffer, size_t imageIndex, VkPipelineLayout layout);

    private:
        void    CreateDescriptorSets();
        void    LoadModel(const std::string& path);
        void    ProcessNode(aiNode* node, const aiScene* scene);
        Mesh*   ProcessMesh(aiMesh* mesh, const aiScene* scene, aiNode* node);
        std::vector<Texture*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string typeName);

    private:
        Device& device;
        std::string name;

        glm::vec3 translation = glm::vec3(1.0f);
        glm::vec3 rotation = glm::vec3(3.1415f);
        glm::vec3 scale = glm::vec3(0.01f);

        std::vector<Mesh*>                  meshes;
        std::vector<VkDescriptorSet>        descriptorSets;
        VkDescriptorSetLayout               descriptorLayout;
        std::vector<Texture*>               texturesLoaded;
        glm::mat4                           pushConstant;
        aiMatrix4x4 currentTransform{};
        std::string path;
    };
}