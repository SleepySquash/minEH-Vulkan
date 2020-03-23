//
//  Vulkan.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 14.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef MH_Vulkan_hpp
#define MH_Vulkan_hpp

#include <optional>
#include <iostream>
#include <vector>
#include <fstream>

#include <vulkan/vulkan.h>

#include "Config.hpp"
#include "Static.hpp"
#include "Vertex.hpp"

namespace mh
{

#pragma mark -
#pragma mark Handy structs

    struct Allocation
    {
        VkDeviceMemory memory = VK_NULL_HANDLE;
        uint32_t offset = 0;
    };

    struct Buffer
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        uint32_t offset = 0;
        Allocation memory;
    };

    struct Image
    {
        uint32_t width, height;
        VkImage image = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        Allocation memory;
    };

    struct Texture
    {
        Image image;
        VkSampler sampler = VK_NULL_HANDLE;
        uint32_t mip = 1;
    };

    struct Descriptor
    {
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VkDescriptorPool pool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> sets = { VK_NULL_HANDLE };
    };

    template<typename T> struct Model
    {
        std::vector<Vertex<T>> vertices;
        std::vector<uint32_t> indices;
        T position{ 0.f }, rotation{ 0.f }, scale{ 1.f };
    };

    struct CameraBufferObject { glm::mat4 view, proj; };
    template<typename T> struct UBO
    {
        uint32_t dirty = 0;
        std::vector<Buffer> buffers;
        T model;
    };

    struct SwapChainProperties
    {
        VkExtent2D extent;
        float aspect;
        VkSurfaceFormatKHR format;
        uint32_t images;
    };

#pragma mark -
#pragma mark Context dependent functions

    

#pragma mark -
#pragma mark Context independent functions
        
    bool isDeviceSuitable(const VkPhysicalDevice& d, const VkSurfaceKHR& surface);
    int ratePhysicalDevice(const VkPhysicalDevice& d);
    
    std::pair<std::optional<uint32_t>, std::optional<uint32_t>> findQueueFamilies(const VkPhysicalDevice& d, const VkSurfaceKHR& surface);
    VkSampleCountFlagBits getMaxUsableSampleCount();
    std::vector<const char*> getRequiredExtensions(const bool& getRequiredExtensions);
    std::vector<char> loadFileInBuffer(const std::string& filename);
    void loadModel(const std::string& MODEL_NAME, std::vector<Vertex<glm::vec3>>& vertices, std::vector<uint32_t>& indices);
    
    bool hasStencilComponent(VkFormat format);
}

#endif /* MH_Vulkan_hpp */
