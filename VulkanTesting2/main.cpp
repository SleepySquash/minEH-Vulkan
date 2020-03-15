//
//  main.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 29.02.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include <iostream>
using std::cout;
using std::endl;

#include <exception>
#include <optional>
#include <fstream>
#include <chrono>

#include <vector>
#include <map>
#include <unordered_map>

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "minEH/Static.hpp"
#include "minEH/Vertex.hpp"
#include "Engine/Mesh2D.hpp"
using namespace mh;

const std::string TEXTURE_NAME = "Data/Textures/texture.jpg";
const std::string MODEL_NAME;
// const std::string TEXTURE_NAME = "Data/Textures/chalet.jpg";
// const std::string MODEL_NAME = "Data/Models/chalet.obj";

struct Application
{
#pragma mark -
#pragma mark -
#pragma mark My stuff
    bool moveA{ false }, moveD{ false }, moveW{ false }, moveS{ false }, moveUp{ false }, moveDown{ false };
    bool rotA{ false }, rotD{ false }, rotW{ false }, rotS{ false };
    
    bool mouseLockEnabled{ false }, firstMouse{ false };
    float lastxpos, lastypos;
    
    /// Camera thanks to: https://learnopengl.com/Getting-started/Camera
    float cameraSpeed{ 0.003f }, cameraSensitivity{ 0.002f }, pitch{ 0 }, yaw{ glm::radians(90.f) };
    glm::vec3 cameraPosition{ 0.f, 0.f, 1.f }, cameraUp{ 0.f, 1.f, 0.f }, cameraDirection{ 0.f, 0.f, 1.f };
    
    mh_obsolete::Mesh2D mesh;
    
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        if (action == GLFW_PRESS)
        {
                 if (key == GLFW_KEY_W) app->moveW = true;
            else if (key == GLFW_KEY_S) app->moveS = true;
            else if (key == GLFW_KEY_D) app->moveD = true;
            else if (key == GLFW_KEY_A) app->moveA = true;
            
            else if (key == GLFW_KEY_E) app->moveUp = true;
            else if (key == GLFW_KEY_Q) app->moveDown = true;
            
            else if (key == GLFW_KEY_UP)    app->rotW = true;
            else if (key == GLFW_KEY_DOWN)  app->rotS = true;
            else if (key == GLFW_KEY_LEFT)  app->rotA = true;
            else if (key == GLFW_KEY_RIGHT) app->rotD = true;
            
            else if (key == GLFW_KEY_ESCAPE)
            {
                if (app->mouseLockEnabled)
                {
                    app->mouseLockEnabled = false; app->firstMouse = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
                else
                {
                    app->mouseLockEnabled = app->firstMouse = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
            }
        }
        else if (action == GLFW_RELEASE)
        {
                 if (key == GLFW_KEY_W) app->moveW = false;
            else if (key == GLFW_KEY_S) app->moveS = false;
            else if (key == GLFW_KEY_D) app->moveD = false;
            else if (key == GLFW_KEY_A) app->moveA = false;
            
            else if (key == GLFW_KEY_E) app->moveUp = false;
            else if (key == GLFW_KEY_Q) app->moveDown = false;
            
            else if (key == GLFW_KEY_UP)    app->rotW = false;
            else if (key == GLFW_KEY_DOWN)  app->rotS = false;
            else if (key == GLFW_KEY_LEFT)  app->rotA = false;
            else if (key == GLFW_KEY_RIGHT) app->rotD = false;
        }
    }
    
    static void cursor_callback(GLFWwindow* window, double xpos, double ypos)
    {
        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        if (!app->mouseLockEnabled) return;
        
        if (app->firstMouse)
        {
            app->lastxpos = xpos;
            app->lastypos = ypos;
            app->firstMouse = false;
        }
      
        float xoffset = xpos - app->lastxpos;
        float yoffset = ypos - app->lastypos;
        app->lastxpos = xpos;
        app->lastypos = ypos;

        xoffset *= app->cameraSensitivity;
        yoffset *= app->cameraSensitivity;

        app->yaw   += xoffset;
        app->pitch += yoffset;

        if(app->pitch > 1.57f) app->pitch = 1.57f;
        if(app->pitch < -1.57f) app->pitch = -1.57f;

        glm::vec3 direction;
        direction.x = cosf(app->yaw) * cosf(app->pitch);
        direction.y = sinf(app->pitch);
        direction.z = sinf(app->yaw) * cosf(app->pitch);
        app->cameraDirection = glm::normalize(direction);
    }
    
    static void mouse_callback(GLFWwindow* window, int button, int action, int mods)
    {
        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        if (action == GLFW_RELEASE && !app->mouseLockEnabled)
        {
            app->mouseLockEnabled = app->firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    
    void InitMyStuff()
    {
        glfwSetInputMode(window, GLFW_CURSOR, mouseLockEnabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        
        glfwSetKeyCallback(window, key_callback);
        glfwSetMouseButtonCallback(window, mouse_callback);
        glfwSetCursorPosCallback(window, cursor_callback);
    }
    
    void UpdateMyStuff(const float& elapsedSeconds)
    {
        if (moveUp)        cameraPosition.y += cameraSpeed * elapsedSeconds;
        else if (moveDown) cameraPosition.y -= cameraSpeed * elapsedSeconds;
        
        /// glm::normalize(glm::vec3( cameraDirection.x, 0.f, cameraDirection.z ))     <- on xz
        if (moveW)      cameraPosition -= cameraDirection * cameraSpeed * elapsedSeconds;
        else if (moveS) cameraPosition += cameraDirection * cameraSpeed * elapsedSeconds;
        if (moveA)      cameraPosition -= glm::normalize(glm::cross(cameraUp, cameraDirection)) * cameraSpeed * elapsedSeconds;
        else if (moveD) cameraPosition += glm::normalize(glm::cross(cameraUp, cameraDirection)) * cameraSpeed * elapsedSeconds;
        
        if (rotA || rotD || rotW || rotS)
        {
            if (rotA)      yaw -= cameraSensitivity * elapsedSeconds;
            else if (rotD) yaw += cameraSensitivity * elapsedSeconds;
            if (rotW)      pitch -= cameraSensitivity * elapsedSeconds;
            else if (rotS) pitch += cameraSensitivity * elapsedSeconds;
            
            cameraDirection.x = cosf(yaw) * cosf(pitch);
            cameraDirection.y = sinf(pitch);
            cameraDirection.z = sinf(yaw) * cosf(pitch);
            cameraDirection = glm::normalize(cameraDirection);
        }
    }
    
    
    
    
#pragma mark -
#pragma mark -
#pragma mark Vulkan
    GLFWwindow* window{ nullptr };
    bool framebufferResized = false, framebufferIconified = false;
    int lastWidth, lastHeight;
    float outputFramerateTimer = 0;
    int outputFramerateCount = 0;
    
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    std::optional<uint32_t> graphicsQueueIndex, presentQueueIndex;
    VkQueue graphicsQueue, presentQueue;
    VkDevice device;
    
    bool anisotropyEnable = false;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    
    VkCommandPool commandPool, shortlivedCommandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    
    VkSwapchainKHR swapchain;
    VkSurfaceFormatKHR swapChainSurfaceFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> framebuffer;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    
    VkShaderModule vertexShader, fragmentShader;
    std::vector<VkSemaphore> renderFinishedSemaphore, imageAvailableSemaphore;
    std::vector<VkFence> inFlightFences, imagesInFlight;
    size_t currentFrame = 0;
    
    // std::vector<Vertex> vertices;
    // std::vector<uint32_t> indices;
    std::vector<Vertex> vertices = {
        { { -0.5f,  -0.5f,   0.0f }, { 1.0f,  0.0f,  0.0f }, { 1.0f,  0.0f } },
        { {  0.5f,  -0.5f,   0.0f }, { 0.0f,  1.0f,  0.0f }, { 0.0f,  0.0f } },
        { {  0.5f,   0.5f,   0.0f }, { 0.0f,  0.0f,  1.0f }, { 0.0f,  1.0f } },
        { { -0.5f,   0.5f,   0.0f }, { 1.0f,  1.0f,  1.0f }, { 1.0f,  1.0f } },
        
        { { -0.5f,  -0.5f,  -0.5f }, { 1.0f,  0.0f,  0.0f }, { 1.0f,  0.0f } },
        { {  0.5f,  -0.5f,  -0.5f }, { 0.0f,  1.0f,  0.0f }, { 0.0f,  0.0f } },
        { {  0.5f,   0.5f,  -0.5f }, { 0.0f,  0.0f,  1.0f }, { 0.0f,  1.0f } },
        { { -0.5f,   0.5f,  -0.5f }, { 1.0f,  1.0f,  1.0f }, { 1.0f,  1.0f } }
    };
    std::vector<uint32_t> indices = {
        0, 1, 2,   2, 3, 0,
        4, 5, 6,   6, 7, 4
    };
    
    VkBuffer vertexBuffer, indexBuffer;
    VkDeviceMemory vertexBufferMemory, indexBufferMemory;
    
    VkDescriptorSetLayout descriptorLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBufferMemory;
    
    VkImage textureImage;
    VkImageView textureImageView;
    VkDeviceMemory textureImageMemory;
    VkSampler textureImageSampler;
    uint32_t textureMipLevels;
    
    VkFormat depthFormat;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    
    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;
    
#pragma mark -
#pragma mark Run()
    void run()
    {
        initGLFW();
        initVulkan();
        
        drawFrame();
        framebufferIconified = glfwGetWindowAttrib(window, GLFW_OCCLUDED);
        while (framebufferIconified) { glfwWaitEvents(); framebufferIconified = glfwGetWindowAttrib(window, GLFW_OCCLUDED); }
        loop();
        
        cleanup();
    }
    
    void initGLFW()
    {
        glfwInit();
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // glfwWindowHint(GLFW_REFRESH_RATE, 60);
        
        window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
        
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        lastWidth = width; lastHeight = height;
        
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetWindowOcclusionCallback(window, framebufferOcclusionCallback);
        
        InitMyStuff();
    }
    
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
    
    static void framebufferIconifyCallback(GLFWwindow* window, int iconified)
    {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = (iconified == 1);
        app->framebufferIconified = iconified;
    }
    
    static void framebufferOcclusionCallback(GLFWwindow* window, int occlusion)
    {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = (occlusion == 1);
        app->framebufferIconified = occlusion;
    }
    
    void initVulkan()
    {
        createInstance();
        setupDebugMessenger();
        pickPhysicalDevice();
        createDevice();
        createCommandPool();
        
        loadShaders();
        
        createSwapchain();
        createColorImage();
        createDepthImage();
        createRenderPass();
        createFramebuffers();
        
        loadModel();
        createTextureImage();
        
        createDescriptorSetLayout();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        
        createGraphicsPipeline();
        
        // dedicateBufferOfVector(vertices.data(), sizeof(vertices[0])*vertices.size(), vertexBuffer, vertexBufferAllocation, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        // dedicateBufferOfVector(indices.data(), sizeof(indices[0])*indices.size(), indexBuffer, indexBufferAllocation, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        createVertexAndIndexBuffers();
        
        mesh.instance = instance;
        mesh.physicalDevice = physicalDevice;
        mesh.device = device;
        mesh.graphicsQueue = graphicsQueue;
        mesh.shortlivedCommandPool = shortlivedCommandPool;
        mesh.swapChainExtent = swapChainExtent;
        mesh.renderPass = renderPass;
        mesh.swapchainImages_size = swapchainImages.size();
        mesh.anisotropyEnable = anisotropyEnable;
        mesh.init();
        
        createCommandBuffer();
        
        createSemaphores();
    }
    
    void loop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            
            std::chrono::high_resolution_clock timer;
            auto start = timer.now();

            drawFrame();
            
            auto stop = timer.now(); outputFramerateCount += 1;
            float deltaTime = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(stop - start).count();
            outputFramerateTimer += deltaTime;
            if (outputFramerateTimer > 400) { cout << outputFramerateCount/outputFramerateTimer * 1000.f << endl;
                outputFramerateCount = 0; outputFramerateTimer = 0.f; }
            
            UpdateMyStuff(deltaTime);
        }
        
        vkDeviceWaitIdle(device);
    }
    
    void recreateSwapchain()
    {
        cout << "recreate try..." << endl;
        
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) { glfwGetFramebufferSize(window, &width, &height); glfwWaitEvents(); }
        while (framebufferIconified) glfwWaitEvents();
        
        vkDeviceWaitIdle(device);
        
        if (lastWidth != width || lastHeight != height)
        {
            cleanupSwapchain();
            
            createSwapchain();
            createColorImage();
            createDepthImage();
            createRenderPass();
            createFramebuffers();
            
            createUniformBuffers();
            createDescriptorPool();
            createDescriptorSets();
            
            mesh.swapChainExtent = swapChainExtent;
            mesh.renderPass = renderPass;
            mesh.swapchainImages_size = swapchainImages.size();
            mesh.recreateSwapchain();
            
            createGraphicsPipeline();
            createCommandBuffer();
            
            lastWidth = width;
            lastHeight = height;
        }
        
        framebufferResized = false;
        
        cout << "done" << endl;
    }
    
    void cleanupSwapchain()
    {
        vkDestroyImage(device, colorImage, nullptr);
        vkDestroyImageView(device, colorImageView, nullptr);
        vkFreeMemory(device, colorImageMemory, nullptr);
        
        vkDestroyImage(device, depthImage, nullptr);
        vkDestroyImageView(device, depthImageView, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);
        
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        for (int i = 0; i < uniformBuffers.size(); ++i)
            cleanBuffer(uniformBuffers[i], uniformBufferMemory[i]);
        
        for (auto& f : framebuffer) vkDestroyFramebuffer(device, f, nullptr);
        vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
        for (auto& imageView : swapchainImageViews) vkDestroyImageView(device, imageView, nullptr);
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }
    
    void cleanBuffer(VkBuffer& buffer, VkDeviceMemory& memory)
    {
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }
    void cleanup()
    {
        cleanupSwapchain();
        mesh.cleanup();
        
        vkDestroyDescriptorSetLayout(device, descriptorLayout, nullptr);
        
        vkDestroySampler(device, textureImageSampler, nullptr);
        vkDestroyImage(device, textureImage, nullptr);
        vkDestroyImageView(device, textureImageView, nullptr);
        vkFreeMemory(device, textureImageMemory, nullptr);
        
        for (int i = 0; i < renderFinishedSemaphore.size(); ++i)
        {
            vkDestroySemaphore(device, renderFinishedSemaphore[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphore[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
        cleanBuffer(vertexBuffer, vertexBufferMemory);
        cleanBuffer(indexBuffer, indexBufferMemory);
        vkDestroyShaderModule(device, vertexShader, nullptr);
        vkDestroyShaderModule(device, fragmentShader, nullptr);
        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyCommandPool(device, shortlivedCommandPool, nullptr);
        vkDestroyDevice(device, nullptr);
        if (mhs::enableValidationLayers)
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
    }
    
    
    
#pragma mark -
#pragma mark Validations layers and Debug
    ///////////////////////////////////////////////////////////////////////////////////////
    /// Thanks to: https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers
    ///////////////////////////////////////////////////////////////////////////////////////
    VkDebugUtilsMessengerEXT debugMessenger;
    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        
        for (const char* layerName : mhs::validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
        
        return false;
    }
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }
    void setupDebugMessenger() {
        if (!mhs::enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
            throw std::runtime_error("failed to set up debug messenger!");
    }
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func) return func(instance, pCreateInfo, pAllocator, pDebugMessenger); else return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func) func(instance, debugMessenger, pAllocator);
    }
    
    
    
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (mhs::enableValidationLayers) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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
    
    VkSampleCountFlagBits getMaxUsableSampleCount()
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);
        
        VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
        /*if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
        if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
        if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
        if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
        if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;*/
        if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
        
        return VK_SAMPLE_COUNT_1_BIT;
    }
    
#pragma mark -
#pragma mark Instance
    void createInstance()
    {
        if (mhs::enableValidationLayers && !checkValidationLayerSupport())
            throw std::runtime_error("validation layers requested, but not available!");
        
        VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
        applicationInfo.pNext = nullptr;
        applicationInfo.pApplicationName = "Name";
        applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.pEngineName = "No";
        applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.apiVersion = VK_API_VERSION_1_0;
        
        VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &applicationInfo;
        
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (mhs::enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(mhs::validationLayers.size());
            createInfo.ppEnabledLayerNames = mhs::validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }
        
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) throw std::runtime_error("createInstance() failed!");
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) throw std::runtime_error("createSurface() failed!");
    }
    
    std::pair<std::optional<uint32_t>, std::optional<uint32_t>> findQueueFamilies(VkPhysicalDevice d)
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
    
    bool isDeviceSuitable(VkPhysicalDevice d)
    {
        // Find support for GRAPHICS and PRESENT queues
        auto families = findQueueFamilies(d);
        
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
    
    int ratePhysicalDevice(VkPhysicalDevice d)
    {
        int score = 0;
        
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(d, &properties);
        
        // If the device is a DISCRETE_GPU then it's a possible winner
        if (properties.deviceType & VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 10000;
        score += properties.limits.maxImageDimension2D;
        
        return score;
    }
    
    void pickPhysicalDevice()
    {
        uint32_t physicalDeviceCount;
        std::vector<VkPhysicalDevice> devices;
        
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
        devices.resize(physicalDeviceCount);
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices.data());
        
        std::multimap<int, VkPhysicalDevice> candidates;
        
        for (const auto& d : devices) {
            if (isDeviceSuitable(d)) {
                candidates.insert({ratePhysicalDevice(d), d});
                break;
            }
        }
        
        if (candidates.size())
        {
            physicalDevice = candidates.begin()->second;
            auto families = findQueueFamilies(physicalDevice);
            
            graphicsQueueIndex = families.first;
            presentQueueIndex = families.second;
            
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(physicalDevice, &features);
            anisotropyEnable = features.samplerAnisotropy;
            msaaSamples = getMaxUsableSampleCount();
            
            return;
        }

        throw std::runtime_error("failed to find a suitable GPU!");
    }

#pragma mark -
#pragma mark Logical Device
    void createDevice()
    {
        float queuePriorities[] = { 1.f };
        
        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        
        VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        queueInfo.pNext = nullptr;
        queueInfo.flags = 0;
        queueInfo.queueFamilyIndex = graphicsQueueIndex.value();
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = queuePriorities;
        queueInfos.push_back(queueInfo);
        
        if (graphicsQueueIndex != presentQueueIndex)
        {
            VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
            queueInfo.pNext = nullptr;
            queueInfo.flags = 0;
            queueInfo.queueFamilyIndex = presentQueueIndex.value();
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = queuePriorities;
            queueInfos.push_back(queueInfo);
        }
        
        VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
        createInfo.pQueueCreateInfos = queueInfos.data();
        if (mhs::enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(mhs::validationLayers.size());
            createInfo.ppEnabledLayerNames = mhs::validationLayers.data();
        } else createInfo.enabledLayerCount = 0;
        std::vector<const char*> extentions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extentions.size());
        createInfo.ppEnabledExtensionNames = extentions.data();
        
        if (anisotropyEnable)
        {
            VkPhysicalDeviceFeatures features = { };
            features.samplerAnisotropy = true;
            features.sampleRateShading = true;
            createInfo.pEnabledFeatures = &features;
        }
        
        if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS) throw std::runtime_error("createDevice() failed!");
        
        // queueIndex is 0 cuz we're only using one queue of this type
        vkGetDeviceQueue(device, graphicsQueueIndex.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, presentQueueIndex.value(), 0, &presentQueue);
    }
    
#pragma mark -
#pragma mark Swap Chain
    VkSurfaceFormatKHR chooseSwapSurfaceFormat()
    {
        uint32_t formatCount;
        std::vector<VkSurfaceFormatKHR> formats;
        
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
        
        for (auto& f : formats)
            if (f.format == VK_FORMAT_R8G8B8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return f;
        
        return formats[0];
    }
    
    VkExtent2D chooseSwapExtent2D(VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;
        else
        {
            VkExtent2D actualExtent;
            
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            
            actualExtent.width = width;
            actualExtent.height = height;
            
            return actualExtent;
        }
    }
    
    VkPresentModeKHR chooseSwapPresentMode()
    {
        uint32_t presentModeCount;
        std::vector<VkPresentModeKHR> presentModes;
        
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
        
        for (auto& p : presentModes)
            if (p == VK_PRESENT_MODE_MAILBOX_KHR) return VK_PRESENT_MODE_MAILBOX_KHR;
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    
    void createSwapchain()
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
        
        swapChainSurfaceFormat = chooseSwapSurfaceFormat();
        swapChainExtent = chooseSwapExtent2D(capabilities);
        
        VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.surface = surface;
        createInfo.minImageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount != 0 && capabilities.maxImageCount < createInfo.minImageCount)
            createInfo.minImageCount = capabilities.maxImageCount;
        createInfo.imageFormat = swapChainSurfaceFormat.format;
        createInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace;
        createInfo.imageExtent = swapChainExtent;
        createInfo.imageArrayLayers = 1; // Amount of layers each image consists of. Always 1 unless you are developing a stereoscopic 3D application.
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Color attachment = directly render to them
        if (presentQueueIndex.value() == graphicsQueueIndex.value())
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            uint32_t families[] = { graphicsQueueIndex.value(), presentQueueIndex.value() };
            createInfo.pQueueFamilyIndices = families;
        }
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = chooseSwapPresentMode();
        createInfo.clipped = true;
        createInfo.oldSwapchain = NULL;
        
        if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapchain) != VK_SUCCESS) throw std::runtime_error("createSwapchain() failed!");
        
        uint32_t imageCount;
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, NULL);
        swapchainImages.resize(imageCount); swapchainImageViews.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());
        
        for (int i = 0; i < imageCount; ++i)
            swapchainImageViews[i] = createImageView(swapchainImages[i], swapChainSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
    
    void createRenderPass()
    {
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // BEFORE the pipeline begins (or AFTER is specified below in dst)
        dependency.dstSubpass = 0; // Our subpass
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // We wait until the image is aquired (COLOR_ATTACHMENT)
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = 0;
        
        
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = swapChainSurfaceFormat.format;
        colorAttachment.samples = msaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = msaaSamples != VK_SAMPLE_COUNT_1_BIT ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // multisampled images cannot be presented directly, need to resolve to a regular image.
        
        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = depthFormat;
        depthAttachment.samples = msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
        VkAttachmentDescription resolveAttachment = {};
        resolveAttachment.format = swapChainSurfaceFormat.format;
        resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        std::vector<VkAttachmentDescription> attachments;
        if (msaaSamples != VK_SAMPLE_COUNT_1_BIT) attachments = { colorAttachment, depthAttachment, resolveAttachment };
        else attachments = { colorAttachment, depthAttachment };
        
        
        VkAttachmentReference colorRef = {};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkAttachmentReference depthRef = {};
        depthRef.attachment = 1;
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
        VkAttachmentReference resolveRef = {};
        resolveRef.attachment = 2;
        resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        
        VkSubpassDescription subpasssDescription = {};
        subpasssDescription.flags = 0;
        subpasssDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasssDescription.inputAttachmentCount = 0;
        subpasssDescription.pInputAttachments = nullptr;
        subpasssDescription.colorAttachmentCount = 1;
        subpasssDescription.pColorAttachments = &colorRef;
        subpasssDescription.pResolveAttachments = msaaSamples != VK_SAMPLE_COUNT_1_BIT ? &resolveRef : nullptr;
        subpasssDescription.pDepthStencilAttachment = &depthRef;
        subpasssDescription.preserveAttachmentCount = 0;
        subpasssDescription.pPreserveAttachments = nullptr;
        
        VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        createInfo.pAttachments = attachments.data();
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpasssDescription;
        createInfo.dependencyCount = 1;
        createInfo.pDependencies = &dependency;
        
        if (vkCreateRenderPass(device, &createInfo, nullptr, &renderPass) != VK_SUCCESS) throw std::runtime_error("createRenderPass() failed!");
    }
    
    void createFramebuffers()
    {
        framebuffer.resize(swapchainImageViews.size());
        
        for (int i = 0; i < swapchainImageViews.size(); ++i)
        {
            std::vector<VkImageView> attachments;
            if (msaaSamples == VK_SAMPLE_COUNT_1_BIT) attachments = { swapchainImageViews[i], depthImageView };
            else attachments = { colorImageView, depthImageView, swapchainImageViews[i] };
            
            VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.renderPass = renderPass;
            createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            createInfo.pAttachments = attachments.data();
            createInfo.width = swapChainExtent.width;
            createInfo.height = swapChainExtent.height;
            createInfo.layers = 1;
            
            if (vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffer[i]) != VK_SUCCESS) throw std::runtime_error("createFremebuffer() failed!");
        }
    }
    
    void loadShaders()
    {
        std::vector<char> vert = loadFileInBuffer("Data/Shaders/spv/shader.vert.spv"),
                          frag = loadFileInBuffer("Data/Shaders/spv/shader.frag.spv");
        
        VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.codeSize = vert.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(vert.data());
        
        if (vkCreateShaderModule(device, &createInfo, nullptr, &vertexShader) != VK_SUCCESS) throw std::runtime_error("loadShaders() failed!");
        
        createInfo.codeSize = frag.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(frag.data());
        
        if (vkCreateShaderModule(device, &createInfo, nullptr, &fragmentShader) != VK_SUCCESS) throw std::runtime_error("loadShaders() failed!");
    }
    
#pragma mark -
#pragma mark Image
    
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
    {
        VkImageCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        createInfo.imageType = VK_IMAGE_TYPE_2D;
        createInfo.format = format;
        createInfo.extent.width = width;
        createInfo.extent.height = height;
        createInfo.extent.depth = 1;
        createInfo.mipLevels = mipLevels;
        createInfo.arrayLayers = 1;
        createInfo.samples = samples;
        createInfo.tiling = tiling;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (vkCreateImage(device, &createInfo, nullptr, &image) != VK_SUCCESS) throw std::runtime_error("createImage() failed!");
        
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);
        
        VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) throw std::runtime_error("createImage() failed!");
        vkBindImageMemory(device, image, imageMemory, 0);
    }
    
    VkImageView createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspect, uint32_t mipLevels)
    {
        VkImageViewCreateInfo viewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewCreateInfo.image = image;
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = format;
        viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.subresourceRange.aspectMask = aspect;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = mipLevels;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;
        
        VkImageView imageView;
        if (vkCreateImageView(device, &viewCreateInfo, nullptr, &imageView) != VK_SUCCESS) throw std::runtime_error("createImageView() failed!");
        return imageView;
    }
    
    void transitionImageLayout(VkImage image, uint32_t mipLevels, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
        
        VkPipelineStageFlags sourceStage, destinationStage;
        
        VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;
        
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else throw std::runtime_error("Specified transitionImageLayout is not supported!");
        
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        
        vkCmdPipelineBarrier(cmdBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        
        endSingleTimeCommands(cmdBuffer);
    }
    
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
        
        VkBufferImageCopy region;
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent.width = width;
        region.imageExtent.height = height;
        region.imageExtent.depth = 1;
        vkCmdCopyBufferToImage(cmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        
        endSingleTimeCommands(cmdBuffer);
    }
    
    void generateMipmaps(VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
        if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) throw std::runtime_error("Format does not support VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT!");
        
        VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
        
        VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        
        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; ++i)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            
            vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            
            VkImageBlit blit;
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            
            vkCmdBlitImage(cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
            
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            
            vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            
            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }
        
        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        
        endSingleTimeCommands(cmdBuffer);
    }
    
    void createTextureImage()
    {
        int width, height, comp;
        stbi_uc* pixels = stbi_load(TEXTURE_NAME.c_str(), &width, &height, &comp, STBI_rgb_alpha);
        textureMipLevels = static_cast<uint32_t>( std::floor( std::log2(std::max(width, height)) ) ) + 1;
        VkDeviceSize imageSize = width * height * 4;
        
        if (!pixels) throw std::runtime_error("createImage() failed to load image!");
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        
        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, imageSize);
        vkUnmapMemory(device, stagingBufferMemory);
        
        stbi_image_free(pixels);
        
        createImage(width, height, textureMipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
        
        transitionImageLayout(textureImage, textureMipLevels, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer, textureImage, width, height);
        generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, width, height, textureMipLevels);
        
        textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, textureMipLevels);
        
        VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.mipLodBias = 0;
        samplerInfo.anisotropyEnable = anisotropyEnable;
        samplerInfo.maxAnisotropy = anisotropyEnable ? 16 : 0;
        samplerInfo.compareEnable = false;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.minLod = 0;
        samplerInfo.maxLod = static_cast<float>(textureMipLevels);
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
        samplerInfo.unnormalizedCoordinates = false;
        if (vkCreateSampler(device, &samplerInfo, nullptr, &textureImageSampler) != VK_SUCCESS) throw std::runtime_error("createImage(1) failed!");
        
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }
    
    void createColorImage()
    {
        createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, swapChainSurfaceFormat.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
        colorImageView = createImageView(colorImage, swapChainSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
    
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) return format;
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) return format;
        }
        throw std::runtime_error("failed to find supported format!");
    }
    
    VkFormat findDepthFormat()
    {
        return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }
    bool hasStencilComponent(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }
    
    void createDepthImage()
    {
        depthFormat = findDepthFormat();
        createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    }
    
#pragma mark -
#pragma mark Descriptor (Uniform Buffer)
    
    void createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboBinding;
        uboBinding.binding = 0;
        uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboBinding.descriptorCount = 1;
        uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        
        VkDescriptorSetLayoutBinding samplerBinding;
        samplerBinding.binding = 1;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.descriptorCount = 1;
        samplerBinding.pImmutableSamplers = nullptr;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        
        std::vector<VkDescriptorSetLayoutBinding> bindings = { uboBinding, samplerBinding };
        VkDescriptorSetLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        createInfo.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorLayout) != VK_SUCCESS) throw std::runtime_error("createDescriptorSetLayout() failed!");
    }
    
    void createUniformBuffers()
    {
        uniformBuffers.resize(swapchainImages.size());
        uniformBufferMemory.resize(swapchainImages.size());
        for (size_t i = 0; i < uniformBuffers.size(); ++i)
            createBuffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uniformBuffers[i], uniformBufferMemory[i]);
    }
    
    void updateUniformBuffers(const uint32_t& imageIndex)
    {
        /*static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();*/
        
        UniformBufferObject ubo;
        // ubo.model = glm::rotate(glm::mat4(1.f), glm::radians(20.f) * time, glm::vec3(0.f, 0.f, 1.f));
        ubo.model = glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
        ubo.view = glm::lookAt(cameraPosition, cameraPosition - cameraDirection /*+ glm::vec3(0.001f, 0.001f, -1.f)*/, cameraUp);
        ubo.proj = glm::perspective(glm::radians(45.f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);
        ubo.proj[1][1] *= -1;
        
        void* data;
        vkMapMemory(device, uniformBufferMemory[imageIndex], 0, sizeof(UniformBufferObject), 0, &data);
            memcpy(data, &ubo, sizeof(UniformBufferObject));
        vkUnmapMemory(device, uniformBufferMemory[imageIndex]);
    }
    
    void createDescriptorPool()
    {
        std::vector<VkDescriptorPoolSize> size(2);
        size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        size[0].descriptorCount = static_cast<uint32_t>(swapchainImages.size());
        size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        size[1].descriptorCount = static_cast<uint32_t>(swapchainImages.size());
        
        VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        createInfo.maxSets = static_cast<uint32_t>(swapchainImages.size());
        createInfo.poolSizeCount = static_cast<uint32_t>(size.size());
        createInfo.pPoolSizes = size.data();
        if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) throw std::runtime_error("createDescriptorPool() failed!");
    }
    void createDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(swapchainImages.size(), descriptorLayout);
        
        VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(swapchainImages.size());
        allocInfo.pSetLayouts = layouts.data();
        descriptorSets.resize(swapchainImages.size());
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) throw std::runtime_error("createDescriptorSets() failed!");
        
        for (size_t i = 0; i < descriptorSets.size(); ++i)
        {
            std::vector<VkWriteDescriptorSet> writeSets(2);
            
            VkDescriptorBufferInfo bufferInfo;
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);
            
            writeSets[0].sType = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeSets[0].dstSet = descriptorSets[i];
            writeSets[0].dstBinding = 0;
            writeSets[0].dstArrayElement = 0;
            writeSets[0].descriptorCount = 1;
            writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeSets[0].pImageInfo = nullptr;
            writeSets[0].pBufferInfo = &bufferInfo;
            writeSets[0].pTexelBufferView = nullptr;
            
            VkDescriptorImageInfo imageInfo;
            imageInfo.sampler = textureImageSampler;
            imageInfo.imageView = textureImageView;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            
            writeSets[1].sType = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeSets[1].dstSet = descriptorSets[i];
            writeSets[1].dstBinding = 1;
            writeSets[1].dstArrayElement = 0;
            writeSets[1].descriptorCount = 1;
            writeSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeSets[1].pImageInfo = &imageInfo;
            writeSets[1].pBufferInfo = nullptr;
            writeSets[1].pTexelBufferView = nullptr;
            
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
        }
    }
    
#pragma mark -
#pragma mark Graphics Pipeline
    void createGraphicsPipeline()
    {
        VkPipelineShaderStageCreateInfo vertexStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        vertexStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexStageInfo.module = vertexShader;
        vertexStageInfo.pName = "main";
        
        VkPipelineShaderStageCreateInfo fragmentStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        fragmentStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentStageInfo.module = fragmentShader;
        fragmentStageInfo.pName = "main";
        
        std::vector<VkPipelineShaderStageCreateInfo> vStages = { vertexStageInfo, fragmentStageInfo };
        
        
        VkVertexInputBindingDescription vBindingDescription = {};
        vBindingDescription.binding = 0;
        vBindingDescription.stride = sizeof(Vertex);
        vBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        std::vector<VkVertexInputAttributeDescription> vAttributeDescription(3);
        vAttributeDescription[0].binding = 0;
        vAttributeDescription[0].location = 0;
        vAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        vAttributeDescription[0].offset = offsetof(Vertex, pos);

        vAttributeDescription[1].binding = 0;
        vAttributeDescription[1].location = 1;
        vAttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        vAttributeDescription[1].offset = offsetof(Vertex, col);
        
        vAttributeDescription[2].binding = 0;
        vAttributeDescription[2].location = 2;
        vAttributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
        vAttributeDescription[2].offset = offsetof(Vertex, texCoords);
        
        VkPipelineVertexInputStateCreateInfo inputStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        inputStateInfo.vertexBindingDescriptionCount = 1;
        inputStateInfo.pVertexBindingDescriptions = &vBindingDescription;
        inputStateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vAttributeDescription.size());
        inputStateInfo.pVertexAttributeDescriptions = vAttributeDescription.data();
        
        
        VkPipelineInputAssemblyStateCreateInfo assemblyStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        assemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        assemblyStateInfo.primitiveRestartEnable = false;
        
        
        /// const VkPipelineTessellationStateCreateInfo*     pTessellationState;
        
        
        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = swapChainExtent.width;
        viewport.height = swapChainExtent.height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1.f;
        
        VkRect2D scissors;
        scissors.offset = { 0, 0 };
        scissors.extent = swapChainExtent;
        
        VkPipelineViewportStateCreateInfo viewportStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        viewportStateInfo.viewportCount = 1;
        viewportStateInfo.pViewports = &viewport;
        viewportStateInfo.scissorCount = 1;
        viewportStateInfo.pScissors = &scissors;
        
        
        VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        rasterizationStateInfo.depthClampEnable = false;
        rasterizationStateInfo.rasterizerDiscardEnable = false;
        rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;//VK_CULL_MODE_BACK_BIT;
        rasterizationStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationStateInfo.depthBiasEnable = false;
        rasterizationStateInfo.depthBiasConstantFactor = 0;
        rasterizationStateInfo.depthBiasClamp = 0;
        rasterizationStateInfo.depthBiasSlopeFactor = 0;
        rasterizationStateInfo.lineWidth = 1.f;
        
        
        VkPipelineMultisampleStateCreateInfo multisampleStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        multisampleStateInfo.rasterizationSamples = msaaSamples;
        multisampleStateInfo.sampleShadingEnable = true;
        multisampleStateInfo.minSampleShading = 1.f;
        multisampleStateInfo.pSampleMask = nullptr;
        multisampleStateInfo.alphaToCoverageEnable = false;
        multisampleStateInfo.alphaToOneEnable = false;
        
        
        VkPipelineDepthStencilStateCreateInfo depthStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStateInfo.depthTestEnable = true;
        depthStateInfo.depthWriteEnable = true;
        depthStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStateInfo.depthBoundsTestEnable = false;
        depthStateInfo.stencilTestEnable = false;
        depthStateInfo.front.failOp = VK_STENCIL_OP_KEEP;
        depthStateInfo.front.passOp = VK_STENCIL_OP_KEEP;
        depthStateInfo.front.compareOp = VK_COMPARE_OP_ALWAYS;
        depthStateInfo.back = depthStateInfo.front;
        depthStateInfo.minDepthBounds = 0;
        depthStateInfo.maxDepthBounds = 1;
        
        
        VkPipelineColorBlendAttachmentState attachmentState = {};
        attachmentState.blendEnable = false;
        attachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        attachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        attachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        attachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        attachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
        attachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        
        VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        colorBlendStateInfo.logicOpEnable = false;
        colorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendStateInfo.attachmentCount = 1;
        colorBlendStateInfo.pAttachments = &attachmentState;
        colorBlendStateInfo.blendConstants[0] = 0.f;
        colorBlendStateInfo.blendConstants[1] = 0.f;
        colorBlendStateInfo.blendConstants[2] = 0.f;
        colorBlendStateInfo.blendConstants[3] = 0.f;
        
        
        VkPipelineDynamicStateCreateInfo dynamicStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        dynamicStateInfo.dynamicStateCount = 0;
        dynamicStateInfo.pDynamicStates = nullptr;
        
        
        VkPipelineLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &descriptorLayout;
        layoutInfo.pushConstantRangeCount = 0;
        layoutInfo.pPushConstantRanges = nullptr;
        
        if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("vkCreatePipelineLayout() failed!");
        
        
        VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        createInfo.stageCount = static_cast<uint32_t>(vStages.size());
        createInfo.pStages = vStages.data();
        createInfo.pVertexInputState = &inputStateInfo;
        createInfo.pInputAssemblyState = &assemblyStateInfo;
        createInfo.pTessellationState = nullptr;
        createInfo.pViewportState = &viewportStateInfo;
        createInfo.pRasterizationState = &rasterizationStateInfo;
        createInfo.pMultisampleState = &multisampleStateInfo;
        createInfo.pDepthStencilState = &depthStateInfo;
        createInfo.pColorBlendState = &colorBlendStateInfo;
        createInfo.pDynamicState = &dynamicStateInfo;
        createInfo.layout = pipelineLayout;
        createInfo.renderPass = renderPass;
        createInfo.subpass = 0;
        createInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        createInfo.basePipelineIndex = -1; // Optional
        
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) throw std::runtime_error("createGraphicsPipeline() failed!");
    }
    
    
#pragma mark -
#pragma mark Buffers (Vertex, Index, Uniform, Command)
    void loadModel()
    {
        if (MODEL_NAME.length() == 0) return;
        
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_NAME.c_str())) throw std::runtime_error("loadModel() failed: " + warn + err);
        
        std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
        
        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex;
                
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoords = {
                          attrib.texcoords[2 * index.texcoord_index + 0],
                    1.f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.col = { 1.0f, 1.0f, 1.0f };
                
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex); }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }
    
    uint32_t findMemoryType(const uint32_t& typeFilter, const VkMemoryPropertyFlags& properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;

        throw std::runtime_error("failed to find suitable memory type!");
    }
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
            throw std::runtime_error("failed to create buffer!");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate buffer memory!");

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }
    
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
        
        VkBufferCopy regions;
        regions.srcOffset = 0;
        regions.dstOffset = 0;
        regions.size = size;
        vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &regions);
        
        endSingleTimeCommands(cmdBuffer);
    }
    
    void createVertexAndIndexBuffers()
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size(),
                     bufferSize2 = sizeof(indices[0]) * indices.size();
        
        VkBuffer stagingBuffer, stagingBuffer2;
        VkDeviceMemory stagingBufferAllocation, stagingBufferAllocation2;
        
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferAllocation);
        createBuffer(bufferSize2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer2, stagingBufferAllocation2);
        
        void* data;
        vkMapMemory(device, stagingBufferAllocation, 0, bufferSize2, 0, &data);
            memcpy(data, vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(device, stagingBufferAllocation);
        
        vkMapMemory(device, stagingBufferAllocation2, 0, bufferSize2, 0, &data);
            memcpy(data, indices.data(), (size_t) bufferSize2);
        vkUnmapMemory(device, stagingBufferAllocation2);
        
        createBuffer(bufferSize2, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
        
        VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
        
            VkBufferCopy regions;
            regions.srcOffset = 0;
            regions.dstOffset = 0;
            regions.size = bufferSize;
            vkCmdCopyBuffer(cmdBuffer, stagingBuffer, vertexBuffer, 1, &regions);
            
            VkBufferCopy regions2;
            regions2.srcOffset = 0;
            regions2.dstOffset = 0;
            regions2.size = bufferSize2;
            vkCmdCopyBuffer(cmdBuffer, stagingBuffer2, indexBuffer, 1, &regions2);
        
        endSingleTimeCommands(cmdBuffer);
        
        cleanBuffer(stagingBuffer, stagingBufferAllocation);
        cleanBuffer(stagingBuffer2, stagingBufferAllocation2);
    }
    
    
    
    
#pragma mark -
#pragma mark CommandBuffer
    
    VkCommandBuffer beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = shortlivedCommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, shortlivedCommandPool, 1, &commandBuffer);
    }
    
    void createCommandPool()
    {
        VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        createInfo.queueFamilyIndex = graphicsQueueIndex.value();
        
        if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS)
            throw std::runtime_error("createCommandPool() failed!");
        
        createInfo.queueFamilyIndex = graphicsQueueIndex.value();
        createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        
        if (vkCreateCommandPool(device, &createInfo, nullptr, &shortlivedCommandPool) != VK_SUCCESS)
            throw std::runtime_error("createCommandPool() failed!");
    }
    
    void createCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocateInfo.commandPool = commandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = static_cast<uint32_t>(framebuffer.size());
        
        commandBuffers.resize(framebuffer.size());
        if (vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers.data()) != VK_SUCCESS) throw std::runtime_error("createCommandPool() failed!");
        
        for (int i = 0; i < framebuffer.size(); ++i)
        {
            VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            beginInfo.pInheritanceInfo = nullptr;
            
            /// Begin DRAW
            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
                throw std::runtime_error("vkBeginCommandBuffer() failed!");
            
            std::vector<VkClearValue> clearValues(2);
            clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
            clearValues[1].depthStencil = { 1.f, 0 };
            VkRenderPassBeginInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = framebuffer[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapChainExtent;
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();
            
            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            
                mesh.draw(commandBuffers[i], i);
            
                vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
                
                VkDeviceSize offset = 0;
                vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, &offset);
                vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
                
                vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
            
                
            
            vkCmdEndRenderPass(commandBuffers[i]);
            
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
               throw std::runtime_error("vkEndCommandBuffer() failed!");
            /// End DRAW
        }
    }
    
    void createSemaphores()
    {
        VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        
        imageAvailableSemaphore.resize(mhs::MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphore.resize(mhs::MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(mhs::MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);
        
        VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        
        for (int i = 0; i < imageAvailableSemaphore.size(); ++i)
        {
            if (vkCreateSemaphore(device, &createInfo, nullptr, &imageAvailableSemaphore[i]) != VK_SUCCESS)
                throw std::runtime_error("vkCreateSemaphore() failed!");
            if (vkCreateSemaphore(device, &createInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS)
                throw std::runtime_error("vkCreateSemaphore() failed!");
            if (vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
               throw std::runtime_error("vkCreateSemaphore() failed!");
        }
    }
    
    
    
    
#pragma mark -
#pragma mark Draw Frame
    void drawFrame()
    {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);
        
        if (result == VK_ERROR_OUT_OF_DATE_KHR) { recreateSwapchain(); return; }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) throw std::runtime_error("vkAcquireNextImageKHR() failed!");
        
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
            vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];
        
        updateUniformBuffers(imageIndex);
        
        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageAvailableSemaphore[currentFrame];
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submitInfo.pWaitDstStageMask = &waitStage;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphore[currentFrame];
        
        vkResetFences(device, 1, &inFlightFences[currentFrame]);
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
            throw std::runtime_error("vkQueueSubmit() failed!");
        
        VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore[currentFrame];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;
        
        result = vkQueuePresentKHR(presentQueue, &presentInfo);
        
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) { framebufferResized = false; recreateSwapchain(); }
        else if (result != VK_SUCCESS) throw std::runtime_error("vkQueuePresentKHR() failed!");
        
        currentFrame = (currentFrame + 1) % mhs::MAX_FRAMES_IN_FLIGHT;
    }
};



#include "minEH/Engine/Engine.hpp"

int main()
{
    mh::Engine app;
    try
    {
        app.Run();
    }
    catch(std::exception& ex) { cout << ex.what() << endl; }
    return 0;
}
