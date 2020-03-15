//
//  Context.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 14.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Context_hpp
#define Context_hpp

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
#include <glm/glm.hpp>

#include "../Static.hpp"
#include "../Vertex.hpp"

#include "Config.hpp"
#include "Vulkan.hpp"
#include "Window.hpp"

namespace mh
{

    namespace exceptions
    {
        struct VK_ERROR_OUT_OF_DATE_KHR : std::exception { virtual const char* what() const throw(); };
    }

#pragma mark -
#pragma mark Context

    struct Context
    {
        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkPhysicalDevice GPU = VK_NULL_HANDLE;
        std::optional<uint32_t> graphicsQueueIndex, presentQueueIndex;
        VkQueue graphicsQueue = VK_NULL_HANDLE, presentQueue = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        
        Window* window = nullptr;
        WindowSize lastSize;
        bool framebufferIconified = false;
        bool framebufferResized = false;
        
        bool anisotropyEnable = false;
        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        
        VkCommandPool commandPool = VK_NULL_HANDLE, shortPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> commandBuffers = { VK_NULL_HANDLE };
        
        VkFormat depthFormat = VK_FORMAT_UNDEFINED;
        Image depthImage, colorImage;
        
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        SwapChainProperties swapChainProps;
        std::vector<VkImage> swapchainImages = { VK_NULL_HANDLE };
        std::vector<VkImageView> swapchainImageViews = { VK_NULL_HANDLE };
        VkRenderPass renderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> framebuffer = { VK_NULL_HANDLE };
        
        std::vector<VkSemaphore> renderFinishedSemaphore = { VK_NULL_HANDLE }, imageAvailableSemaphore = { VK_NULL_HANDLE };
        std::vector<VkFence> inFlightFences = { VK_NULL_HANDLE }, imagesInFlight = { VK_NULL_HANDLE };
        size_t currentFrame = 0;
        
#pragma mark -
#pragma mark Init/Destroy
        
        void create(Window& window);
        void recreate();
        void destroy();
        
#pragma mark -
#pragma mark Instance
        
        void createInstance();
        void createSurface(Window& window);
        void pickPhysicalDevice();
        void createDevice();
        
#pragma mark -
#pragma mark Validations layers and Debug
        
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        bool checkValidationLayerSupport();
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void setupDebugMessenger();
        VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
        
#pragma mark -
#pragma mark SwapChain, depth, renderpass, framebuffer
        
        VkSurfaceFormatKHR chooseSwapSurfaceFormat();
        VkExtent2D chooseSwapExtent2D(VkSurfaceCapabilitiesKHR& capabilities, uint32_t windowWidth, uint32_t windowHeight); // glfwGetFramebufferSize(window, &w, &h);
        VkPresentModeKHR chooseSwapPresentMode();
        
        void createSwapchain();
        
        void createColorImage();
        void createDepthImage();
        
        void createRenderPass();
        void createFramebuffers();
        
#pragma mark -
#pragma mark Semaphores
        
        void createSemaphores();
        
        // textures and models
        // descriptors
        // graphicsPipeline and draw submits
        VkShaderModule loadShader(const std::string& path);
        
#pragma mark -
#pragma mark Command buffer
        
        void createCommandPool();
        void createCommandBuffer();
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer& commandBuffer);
        
        void beginRecord();
        void endRecord();
        
#pragma mark -
#pragma mark Allocations
        
        void freeTexture(Texture& texture);
        void freeImage(Image& image);
        void freeBuffer(Buffer& buffer);
        void freeDescriptor(Descriptor& descriptor);
        
#pragma mark -
#pragma mark Buffer
        
        uint32_t findMemoryType(const uint32_t& typeFilter, const VkMemoryPropertyFlags& properties);
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        
#pragma mark -
#pragma mark Image
        
        void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        VkImageView createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspect, uint32_t mipLevels);
        void transitionImageLayout(VkImage image, uint32_t mipLevels, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void generateMipmaps(VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        
#pragma mark -
#pragma mark Supporting functions
        
        VkSampleCountFlagBits getMaxUsableSampleCount();
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat findDepthFormat();
        
#pragma mark -
#pragma mark Draw
        
        void draw();
        
    };
}

#endif /* Context_hpp */
