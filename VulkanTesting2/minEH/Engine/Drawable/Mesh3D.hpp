//
//  Mesh.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 18.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Mesh_hpp
#define Mesh_hpp

#include <iostream>
#include <fstream>
#include <vector>

#include "Drawable.hpp"
#include "Camera.hpp"

namespace mh
{
    struct Mesh3D : Drawable
    {
        uint32_t maxMipLevels = 0;
        
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;
        
        VkShaderModule vertexShader = VK_NULL_HANDLE,
                       fragmentShader = VK_NULL_HANDLE;
        
        Buffer vertexBuffer, indexBuffer;
        Descriptor descriptor;
        Texture texture;
        
        UBO<glm::mat4> ubo;
        Model<glm::vec3> model;
        
        Mesh3D(Context& context, CameraView& cameraView);
        ~Mesh3D();
        void init();
        void recreate() override;
        void recreateUniformBuffers();
        void createUniformBuffers();
        void createDescriptorSetLayout();
        void createDescriptors();
        void createGraphicsPipeline();
        
        void onRecord(VkCommandBuffer& commandBuffer, uint32_t i) override;
        void updateUniformBuffers(const uint32_t& i, const bool& force = false) override;
        
        void dirty();
        void update(const float& elapsedTime) override;
        
        void loadTexture(const std::string& textureName);
        void loadModel(const std::string& modelName);
        
        void setPosition(const glm::vec3& pos);
        void setRotation(const glm::vec3& rot);
        void setScale(const glm::vec3& scl);
        const glm::vec3& getPosition();
        const glm::vec3& getRotation();
        const glm::vec3& getScale();
    };
}

#endif /* Mesh_hpp */
