//
//  ObjectContext.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 14.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef ObjectContext_hpp
#define ObjectContext_hpp

#include <iostream>
using std::cout;
using std::endl;

#include <exception>
#include <optional>
#include <fstream>

#include <vector>
#include <map>
#include <unordered_map>

#include <vulkan/vulkan.h>
#include <glm/fwd.hpp>

#include "Vertex.hpp"
#include "Context.hpp"

namespace mh
{

#pragma mark -
#pragma mark PerObject
    struct ObjectContext
    {
        Context* context = nullptr;
        
        std::vector<Vertex<glm::vec3>> vertices;
        std::vector<uint32_t> indices;
        
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;
        
        VkShaderModule vertexShader = VK_NULL_HANDLE,
                       fragmentShader = VK_NULL_HANDLE;
        
        Buffer vertexBuffer, indexBuffer;
        
        VkDescriptorSetLayout descriptorLayout;
        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;
        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBufferMemory;
        
        Texture texture;
        
        void loadModel(const std::string& path);
    };
}

#endif /* ObjectContext_hpp */
