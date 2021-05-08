#include "Model.h"
#include <filesystem>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/Application.h"

namespace SnowEngine {
    Model::Model(Device& device, const std::string& modelPath) : device(device) {
        auto abs = std::filesystem::absolute(modelPath);
        name = abs.filename().string();
        std::stringstream ss;
        ss.str(name);
        std::getline(ss, name, '.');
        LoadModel(abs.string());
        CreateDescriptorSets();
    }

    Model::~Model() {
        vkDestroyDescriptorSetLayout(device, descriptorLayout, nullptr);

        for (Texture* texture : texturesLoaded)
            delete texture;

        for (auto mesh : meshes)
            delete mesh;
    }

    void Model::Draw(VkCommandBuffer commandBuffer, size_t imageIndex, VkPipelineLayout layout) {
        for (auto& mesh : meshes) {
            std::vector<VkDescriptorSet> sets = { mesh->GetDescriptorSet(imageIndex) };
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, sets.size(), sets.data(), 0, nullptr);
            
            mesh->Draw(commandBuffer); 
        }
    }
    
	void Model::DrawShadow(VkCommandBuffer commandBuffer) {
        for (auto& mesh : meshes) {
            mesh->Draw(commandBuffer);
        }
	}

	void Model::CreateDescriptorSets() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = { meshes[0]->GetLayoutBindings() };

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device.GetDevice(), &layoutInfo, nullptr, &descriptorLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create descriptor set layout!");

        for (Mesh* mesh : meshes) {
            mesh->CreateDescriptorSet(descriptorLayout);
        }
    }

    void Model::LoadModel(const std::string& path) {
        this->path = std::filesystem::absolute(path).remove_filename().string();
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "[Assimp]: " << importer.GetErrorString() << std::endl;
            return;
        }
       
        ProcessNode(scene->mRootNode, scene); //basta una call perchè la funzione è recursiva
    }  

    void Model::ProcessNode(aiNode* node, const aiScene* scene) {
        for (size_t i = 0; i < node->mNumMeshes; i++) { //processo tutti i mesh nel nodo
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(ProcessMesh(mesh, scene, node));
        }

        for (size_t i = 0; i < node->mNumChildren; i++)
            ProcessNode(node->mChildren[i], scene); //se ci sono altri nodi nella scena li processo
    }

    Mesh* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, aiNode* node) {
        Mesh::ProcessingFlags flags;

        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;
        std::vector<Texture*> textures;

        vertices.reserve(mesh->mNumVertices);
        Vertex vertex;
        for (size_t i = 0; i < mesh->mNumVertices; i++) {
            vertex.pos.x = mesh->mVertices[i].x;
            vertex.pos.y = mesh->mVertices[i].y;
            vertex.pos.z = mesh->mVertices[i].z;

            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;

            if (mesh->mTextureCoords[0]) { //se ha delle coordinate
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.texCoord = vec;
            }
            else
                vertex.texCoord = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        for (size_t i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; j++)
                indices.push_back(static_cast<uint16_t>(face.mIndices[j]));
        }

        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            if (float value = material->Get(AI_MATKEY_OPACITY, value) == AI_SUCCESS && value < 1.0f)
                flags.HasTransparency = true;     

            std::vector<Texture*> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "textureDiffuse", 1);
            if(diffuseMaps.size() == 0)
                textures.push_back(new Texture(device, VK_SHADER_STAGE_FRAGMENT_BIT, 1, "resources/textures/white.png"));
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            std::vector<Texture*> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "textureSpecular", 2);
			if (specularMaps.size() == 0)
				textures.push_back(new Texture(device, VK_SHADER_STAGE_FRAGMENT_BIT, 2, "resources/textures/white.png"));
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

            std::vector<Texture*> normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, "textureNormals", 3);
			if (normalMaps.size() == 0)
				textures.push_back(new Texture(device, VK_SHADER_STAGE_FRAGMENT_BIT, 3, "resources/textures/white.png"));
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        }

        return new Mesh{ device, vertices, indices, textures, flags };
    }


    std::vector<Texture*> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string typeName, uint32_t binding)
    {
        std::vector<Texture*> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            for (unsigned int j = 0; j < texturesLoaded.size(); j++) {
                if (std::strcmp(texturesLoaded[j]->GetPath().data(), str.C_Str()) == 0) {
                    textures.push_back(texturesLoaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                Texture* texture = new Texture(device, VK_SHADER_STAGE_FRAGMENT_BIT, binding, path + str.C_Str());
                textures.push_back(texture);
                texturesLoaded.push_back(texture);
            }    
            return textures;
        }     
    }
}