//
//  Vulkan.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 12.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Vulkan_hpp
#define Vulkan_hpp

#include <iostream>
using std::cout;
using std::endl;

#include <exception>
#include <optional>
#include <fstream>

#include <vector>
#include <map>
#include <unordered_map>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "../minEH/Static.hpp"
#include "../minEH/Vertex.hpp"

namespace mh_obsolete
{
#pragma mark -
#pragma mark Validations layers and Debug

    ///////////////////////////////////////////////////////////////////////////////////////
    /// Thanks to: https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers
    ///////////////////////////////////////////////////////////////////////////////////////
    bool checkValidationLayerSupport();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void setupDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger);
    
#pragma mark -
#pragma mark Supporting functions

    void cleanBuffer(VkDevice& device, VkBuffer& buffer, VkDeviceMemory& memory);
    std::vector<const char*> getRequiredExtensions();
    std::vector<char> loadFileInBuffer(const std::string& filename);
    uint32_t findMemoryType(VkPhysicalDevice& physicalDevice, const uint32_t& typeFilter, const VkMemoryPropertyFlags& properties);
    VkCommandBuffer beginSingleTimeCommands(VkDevice& device, VkCommandPool commandPool);
    void endSingleTimeCommands(VkDevice& device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer);
    VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice& physicalDevice);
    VkFormat findSupportedFormat(VkPhysicalDevice& physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat(VkPhysicalDevice& physicalDevice);
    bool hasStencilComponent(VkFormat format);
    
#pragma mark -
#pragma mark Instance

    VkInstance createInstance();
    VkSurfaceKHR createSurface(VkInstance& instance, GLFWwindow* window);
    std::pair<std::optional<uint32_t>, std::optional<uint32_t>> findQueueFamilies(VkPhysicalDevice d, VkSurfaceKHR& surface);
        
    bool isDeviceSuitable(VkPhysicalDevice d, VkSurfaceKHR& surface);
    int ratePhysicalDevice(VkPhysicalDevice d);
    
#pragma mark -
#pragma mark Swap Chain

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
    VkExtent2D chooseSwapExtent2D(VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
    VkPresentModeKHR chooseSwapPresentMode(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
    VkShaderModule loadShader(VkDevice& device, const std::string& path);
    
#pragma mark -
#pragma mark Image
    
    void createImage(VkDevice& device, VkPhysicalDevice& physicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkDevice& device, VkImage& image, VkFormat format, VkImageAspectFlags aspect, uint32_t mipLevels);
    void transitionImageLayout(VkDevice& device, VkCommandPool& commandPool, VkQueue& queue, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void copyBufferToImage(VkDevice& device, VkCommandPool& commandPool, VkQueue& queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void generateMipmaps(VkDevice& device, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& queue, VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels);
    
#pragma mark -
#pragma mark Buffers (Vertex, Index, Uniform, Command)

    void loadModel(const std::string& MODEL_NAME, std::vector<mh::Vertex>& vertices, std::vector<uint32_t> indices);
    void createBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkDevice& device, VkCommandPool& commandPool, VkQueue queue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    
#pragma mark -
#pragma mark CommandBuffer

    VkCommandPool createCommandPool(VkDevice& device, uint32_t queueIndex, VkCommandPoolCreateFlags flags);

}
#endif /* Vulkan_hpp */
