//
//  Sprite.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 18.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Sprite_hpp
#define Sprite_hpp

#include <iostream>
#include <vector>

#include "Drawable.hpp"

namespace mh
{
    struct Mesh2D : Drawable
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
        Model<glm::vec2> model;
        
        Mesh2D(Context& context);
        ~Mesh2D();
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
        
        void setPosition(const glm::vec2& pos);
        void setRotation(const float& rot);
        void setScale(const glm::vec2& scl);
        const glm::vec2& getPosition();
        const float& getRotation();
        const glm::vec2& getScale();
    };
}

#endif /* Sprite_hpp */
