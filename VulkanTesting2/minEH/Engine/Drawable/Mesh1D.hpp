//
//  Mesh1D.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 23.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Mesh1D_hpp
#define Mesh1D_hpp

#include <iostream>
#include <vector>

#include "Drawable.hpp"

namespace mh
{
    struct Mesh1D : Drawable
    {
        uint32_t maxMipLevels = 0;
        
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;
        
        VkShaderModule vertexShader = VK_NULL_HANDLE,
                       fragmentShader = VK_NULL_HANDLE;
        
        Buffer vertexBuffer, indexBuffer;
        Descriptor descriptor;
        Texture texture;
        
        Model<glm::vec2> model;
        
        Mesh1D(Context& context);
        ~Mesh1D();
        void init();
        void recreate() override;
        void createDescriptorSetLayout();
        void createDescriptors();
        void createGraphicsPipeline();
        
        void onRecord(VkCommandBuffer& commandBuffer, uint32_t i) override;
        
        void loadTexture(const std::string& textureName);
    };
}

#endif /* Mesh1D_hpp */
