//
//  Vulkan.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 14.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Vulkan.hpp"

#include <unordered_map>
#include <tiny_obj_loader.h>

namespace mh
{
#pragma mark -
#pragma mark Supporting functions
    
    bool isDeviceSuitable(const VkPhysicalDevice& d, const VkSurfaceKHR& surface)
    {
        // Find support for GRAPHICS and PRESENT queues
        auto families = findQueueFamilies(d, surface);
        
        if (families.first.has_value() && families.second.has_value())
        {
            // Now let's look if VK_KHR_SWAPCHAIN_EXTENSION_NAME is supported by this device
            
            uint32_t propertyCount;
            std::vector<VkExtensionProperties> extensions;
            
            vkEnumerateDeviceExtensionProperties(d, NULL, &propertyCount, NULL);
            extensions.resize(propertyCount);
            vkEnumerateDeviceExtensionProperties(d, NULL, &propertyCount, extensions.data());
            
            bool support = false;
            for (auto& e : extensions)
                if (std::string(e.extensionName) == VK_KHR_SWAPCHAIN_EXTENSION_NAME) { support = true; break; }
            
            if (support)
            {
                // Let's check if the device has at least one format and present mode.
                uint32_t formatCount;
                vkGetPhysicalDeviceSurfaceFormatsKHR(d, surface, &formatCount, NULL);
                
                uint32_t modesCount;
                vkGetPhysicalDeviceSurfacePresentModesKHR(d, surface, &modesCount, NULL);
                
                return formatCount > 0 && modesCount > 0;
            }
        }
        
        return false;
    }
    
    int ratePhysicalDevice(const VkPhysicalDevice& d)
    {
        int score = 0;
        
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(d, &properties);
        
        // If the device is a DISCRETE_GPU then it's a possible winner
        if (properties.deviceType & VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 10000;
        score += properties.limits.maxImageDimension2D;
        
        return score;
    }


    std::pair<std::optional<uint32_t>, std::optional<uint32_t>> findQueueFamilies(const VkPhysicalDevice& d, const VkSurfaceKHR& surface)
    {
        // List all the queue families supported by this device
        uint32_t queueFamilyPropertyCount;
        std::vector<VkQueueFamilyProperties> queueFamilyProperties;
        
        vkGetPhysicalDeviceQueueFamilyProperties(d, &queueFamilyPropertyCount, NULL);
        queueFamilyProperties.resize(queueFamilyPropertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(d, &queueFamilyPropertyCount, queueFamilyProperties.data());
        
        // Find GRAPHICS queue
        std::optional<uint32_t> graphicsQueue, presentQueue;
        for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
            if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsQueue = i; break;
            }
        
        // Find PRESENT queue
        for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
            VkBool32 supported;
            vkGetPhysicalDeviceSurfaceSupportKHR(d, i, surface, &supported);
            if (supported) {
                presentQueue = i; break;
            }
        }
        
        return std::make_pair(graphicsQueue, presentQueue);
    }

    std::vector<const char*> getRequiredExtensions(const bool& enableValidationLayers)
    {
#ifdef MINEH_WINDOW_API_GLFW
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#endif
        if (enableValidationLayers) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    std::vector<char> loadFileInBuffer(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) throw std::runtime_error("loadFileInBuffer() failed: " + filename);
        
        size_t size = file.tellg();
        std::vector<char> data(size);
        file.seekg(0);
        file.read(data.data(), size);
        
        return data;
    }

    void loadModel(const std::string& MODEL_NAME, std::vector<Vertex<glm::vec3>>& vertices, std::vector<uint32_t>& indices)
    {
        if (MODEL_NAME.length() == 0) return;
        
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_NAME.c_str())) throw std::runtime_error("loadModel() failed: " + warn + err);
        
        std::unordered_map<Vertex<glm::vec3>, uint32_t> uniqueVertices = {};
        
        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex<glm::vec3> vertex;
                
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.uv = {
                          attrib.texcoords[2 * index.texcoord_index + 0],
                    1.f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
                
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex); }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    } 

    bool hasStencilComponent(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }
}
