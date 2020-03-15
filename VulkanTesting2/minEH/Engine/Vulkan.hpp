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

namespace mh
{

#pragma mark -
#pragma mark Handy structs

    struct Allocation
    {
        VkDeviceMemory memory = VK_NULL_HANDLE;
        uint32_t memoryOffset = 0;
    };

    struct Buffer
    {
        VkBuffer buffer = VK_NULL_HANDLE;
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

    struct SwapChainProperties
    {
        VkExtent2D extent;
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
    std::vector<const char*> getRequiredExtensions();
    std::vector<char> loadFileInBuffer(const std::string& filename);
    
    bool hasStencilComponent(VkFormat format);
}

#endif /* MH_Vulkan_hpp */
