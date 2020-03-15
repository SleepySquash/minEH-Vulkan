//
//  Mesh2D.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 11.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Mesh2D_hpp
#define Mesh2D_hpp

#include <iostream>
#include <fstream>

#include <vector>
#include <unordered_map>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace mh_obsolete
{
    struct Vertex2D
    {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    struct Mesh2D
    {
        const std::string TEXTURE_NAME = "Data/Textures/79989656_p0.png";
        
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkQueue graphicsQueue;
        VkCommandPool shortlivedCommandPool;
        VkRenderPass renderPass;
        VkExtent2D swapChainExtent;
        size_t swapchainImages_size;
        bool anisotropyEnable;
        
        
        VkShaderModule vertexShader, fragmentShader;
        
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureImageSampler;
        uint32_t textureMipLevels;
        
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;
        
        /*std::vector<Vertex2D> vertices = {
            { { -0.8f,  -0.9f }, { 0.0f,  0.0f } },
            { { -0.5f,  -0.9f }, { 1.0f,  0.0f } },
            { { -0.5f,  -0.3f }, { 1.0f,  1.0f } },
            { { -0.8f,  -0.3f }, { 0.0f,  1.0f } },
        };
        std::vector<uint32_t> indices = {
            0, 1, 2,   2, 3, 0,
        };*/
        std::vector<Vertex2D> vertices = {
            { { -1.f,  -1.f }, { 0.0f,  0.0f } },
            { {  1.f,  -1.f }, { 1.0f,  0.0f } },
            { { -1.f,   1.f }, { 0.0f,  1.0f } },
            { {  1.f,   1.f }, { 1.0f,  1.0f } }
        };
        std::vector<uint32_t> indices = {
            0, 1, 2,   1, 3, 2
        };
        
        VkBuffer vertexBuffer, indexBuffer;
        VkDeviceMemory vertexBufferMemory, indexBufferMemory;
        
        VkDescriptorSetLayout descriptorLayout;
        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;
        
        
        
        void init();
        void draw(VkCommandBuffer& commandBuffer, const uint32_t& i);
        
        void cleanupSwapchain();
        void recreateSwapchain();
        void cleanup();
        
        
        void createGraphicsPipeline();
        void createVertexAndIndexBuffers();
        void createTexture();
        
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSets();
        
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    };
}

#endif /* Mesh2D_hpp */
