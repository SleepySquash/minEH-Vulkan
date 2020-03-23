//
//  Context.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 14.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Context.hpp"

#include <stb_image.h>

namespace mh
{

    namespace exceptions
    {
        const char* VK_ERROR_OUT_OF_DATE_KHR::what() const throw() { return "VK_ERROR_OUT_OF_DATE_KHR"; }
    }

#pragma mark -
#pragma mark Init/Destroy
        
    void Context::create(Window& window)
    {
        createInstance();
        createSurface(window);
        setupDebugMessenger();
        pickPhysicalDevice();
        createDevice();
        
        createCommandPool();
        
        createSwapchain();
        createColorImage();
        createDepthImage();
        createRenderPass();
        createFramebuffers();
        
        createSemaphores();
        createCommandBuffer();
    }
    void Context::recreate()
    {
        cout << "recreate try..." << endl;
        
        vkDeviceWaitIdle(device);
        
        WindowSize size = window->getSize();
        if (lastSize.width != size.width || lastSize.height != size.height)
        {
            freeImage(colorImage);
            freeImage(depthImage);
            
            for (auto& f : framebuffer) vkDestroyFramebuffer(device, f, nullptr);
            vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
            vkDestroyRenderPass(device, renderPass, nullptr);
            for (auto& imageView : swapchainImageViews) vkDestroyImageView(device, imageView, nullptr);
            // vkDestroySwapchainKHR(device, swapchain, nullptr);
            
            createSwapchain();
            createColorImage();
            createDepthImage();
            createRenderPass();
            createFramebuffers();
            createCommandBuffer();
            
            lastSize.width = size.width;
            lastSize.height = size.height;
        }
        
        cout << "done" << endl;
    }
    void Context::destroy()
    {
        freeImage(colorImage);
        freeImage(depthImage);
        
        for (auto& f : framebuffer) vkDestroyFramebuffer(device, f, nullptr);
        vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        vkDestroyRenderPass(device, renderPass, nullptr);
        for (auto& imageView : swapchainImageViews) vkDestroyImageView(device, imageView, nullptr);
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        
        
        for (int i = 0; i < renderFinishedSemaphore.size(); ++i)
        {
            vkDestroySemaphore(device, renderFinishedSemaphore[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphore[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyCommandPool(device, shortPool, nullptr);
        vkDestroyDevice(device, nullptr);
        if (enableValidationLayers)
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

#pragma mark -
#pragma mark Instance and Device

    void Context::createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport())
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
        
        auto extensions = getRequiredExtensions(enableValidationLayers);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }
        
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) throw std::runtime_error("createInstance() failed!");
    }

    void Context::createSurface(Window& window)
    {
        this->window = &window;
#if defined(MINEH_WINDOW_API_GLFW)
        if (glfwCreateWindowSurface(instance, window.window, nullptr, &surface) != VK_SUCCESS) throw std::runtime_error("createSurface() failed!");
#endif
    }

    void Context::pickPhysicalDevice()
    {
        uint32_t physicalDeviceCount;
        std::vector<VkPhysicalDevice> devices;
        
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
        devices.resize(physicalDeviceCount);
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices.data());
        
        std::multimap<int, VkPhysicalDevice> candidates;
        
        for (const auto& d : devices)
            if (isDeviceSuitable(d, surface)) {
                candidates.insert({ratePhysicalDevice(d), d}); break; }
        
        if (candidates.size())
        {
            GPU = candidates.begin()->second;
            auto families = findQueueFamilies(GPU, surface);
            
            graphicsQueueIndex = families.first;
            presentQueueIndex = families.second;
            
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(GPU, &features);
            anisotropyEnable = features.samplerAnisotropy;
            msaaSamples = getMaxUsableSampleCount();
            
            return;
        }

        throw std::runtime_error("failed to find a suitable GPU!");
    }

    void Context::createDevice()
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
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
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
        
        if (vkCreateDevice(GPU, &createInfo, NULL, &device) != VK_SUCCESS) throw std::runtime_error("createDevice() failed!");
        
        // queueIndex is 0 cuz we're only using one queue of this type
        vkGetDeviceQueue(device, graphicsQueueIndex.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, presentQueueIndex.value(), 0, &presentQueue);
    }
    
#pragma mark -
#pragma mark Validations layers and Debug
    ///////////////////////////////////////////////////////////////////////////////////////
    /// Thanks to: https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers
    ///////////////////////////////////////////////////////////////////////////////////////
    bool Context::checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        
        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers)
                if (strcmp(layerName, layerProperties.layerName) == 0) { layerFound = true; break; }
            if (!layerFound) return false;
        }

        return true;
        
        return false;
    }
    VKAPI_ATTR VkBool32 VKAPI_CALL Context::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) { std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl; return VK_FALSE; }
    void Context::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }
    void Context::setupDebugMessenger()
    {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
            throw std::runtime_error("failed to set up debug messenger!");
    }
    VkResult Context::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func) return func(instance, pCreateInfo, pAllocator, pDebugMessenger); else return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    void Context::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func) func(instance, debugMessenger, pAllocator);
    }

#pragma mark -
#pragma mark Swapchain

    VkSurfaceFormatKHR Context::chooseSwapSurfaceFormat()
    {
        uint32_t formatCount;
        std::vector<VkSurfaceFormatKHR> formats;
        
        vkGetPhysicalDeviceSurfaceFormatsKHR(GPU, surface, &formatCount, NULL);
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(GPU, surface, &formatCount, formats.data());
        
        for (auto& f : formats)
            if (f.format == VK_FORMAT_R8G8B8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return f;
        
        return formats[0];
    }
    
    VkExtent2D Context::chooseSwapExtent2D(VkSurfaceCapabilitiesKHR& capabilities, uint32_t windowWidth, uint32_t windowHeight)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;
        else
        {
            VkExtent2D actualExtent;
            // glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
            
            actualExtent.width = windowWidth;
            actualExtent.height = windowHeight;
            
            return actualExtent;
        }
    }
    
    VkPresentModeKHR Context::chooseSwapPresentMode()
    {
        uint32_t presentModeCount;
        std::vector<VkPresentModeKHR> presentModes;
        
        vkGetPhysicalDeviceSurfacePresentModesKHR(GPU, surface, &presentModeCount, nullptr);
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(GPU, surface, &presentModeCount, presentModes.data());
        
        for (auto& p : presentModes)
            if (p == VK_PRESENT_MODE_MAILBOX_KHR) return VK_PRESENT_MODE_MAILBOX_KHR;
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    
    void Context::createSwapchain()
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GPU, surface, &capabilities);
        
        swapChainProps.format = chooseSwapSurfaceFormat();
        swapChainProps.extent = chooseSwapExtent2D(capabilities, 0, 0);
        swapChainProps.aspect = (float)swapChainProps.extent.width/swapChainProps.extent.height;
        
        VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.surface = surface;
        createInfo.minImageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount != 0 && capabilities.maxImageCount < createInfo.minImageCount)
            createInfo.minImageCount = capabilities.maxImageCount;
        createInfo.imageFormat = swapChainProps.format.format;
        createInfo.imageColorSpace = swapChainProps.format.colorSpace;
        createInfo.imageExtent = swapChainProps.extent;
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
        createInfo.oldSwapchain = swapchain;
        
        if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapchain) != VK_SUCCESS) throw std::runtime_error("createSwapchain() failed!");
        
        vkGetSwapchainImagesKHR(device, swapchain, &swapChainProps.images, NULL);
        swapchainImages.resize(swapChainProps.images); swapchainImageViews.resize(swapChainProps.images);
        vkGetSwapchainImagesKHR(device, swapchain, &swapChainProps.images, swapchainImages.data());
        
        for (int i = 0; i < swapChainProps.images; ++i)
            swapchainImageViews[i] = createImageView(swapchainImages[i], swapChainProps.format.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }

#pragma mark -
#pragma mark Depth and MSAA

    void Context::createColorImage()
    {
        createImage(swapChainProps.extent.width, swapChainProps.extent.height, 1, msaaSamples, swapChainProps.format.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage.image, colorImage.memory.memory);
        colorImage.view = createImageView(colorImage.image, swapChainProps.format.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
    void Context::createDepthImage()
    {
        depthFormat = findDepthFormat();
        createImage(swapChainProps.extent.width, swapChainProps.extent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage.image, depthImage.memory.memory);
        depthImage.view = createImageView(depthImage.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    }

#pragma mark -
#pragma mark RenderPass and Framebuffers

    void Context::createRenderPass()
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
        colorAttachment.format = swapChainProps.format.format;
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
        resolveAttachment.format = swapChainProps.format.format;
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
    
    void Context::createFramebuffers()
    {
        framebuffer.resize(swapchainImageViews.size());
        
        for (int i = 0; i < swapchainImageViews.size(); ++i)
        {
            std::vector<VkImageView> attachments;
            if (msaaSamples == VK_SAMPLE_COUNT_1_BIT) attachments = { swapchainImageViews[i], depthImage.view };
            else attachments = { colorImage.view, depthImage.view, swapchainImageViews[i] };
            
            VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.renderPass = renderPass;
            createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            createInfo.pAttachments = attachments.data();
            createInfo.width = swapChainProps.extent.width;
            createInfo.height = swapChainProps.extent.height;
            createInfo.layers = 1;
            
            if (vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffer[i]) != VK_SUCCESS) throw std::runtime_error("createFremebuffer() failed!");
        }
    }
    
#pragma mark -
#pragma mark Semaphores

    void Context::createSemaphores()
    {
        VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        
        imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
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
    
    // textures and models
    // descriptors
    // graphicsPipeline and draw submits

    VkShaderModule Context::loadShader(const std::string& path)
    {
        std::vector<char> shader = loadFileInBuffer(path);
        
        VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        createInfo.codeSize = shader.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.data());
        
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) throw std::runtime_error("loadShaders() failed!");
        return shaderModule;
    }

    std::pair<VkPipeline, VkPipelineLayout> Context::generateDefaultPipeline(VkShaderModule& vertexShader, VkShaderModule& fragmentShader, std::vector<VkVertexInputAttributeDescription>& vAttributeDescription, VkVertexInputBindingDescription& vBindingDescription, Descriptor* descriptor, bool depthEnabled, const VkPolygonMode& polygonMode, const VkCullModeFlags& cullMode, const VkFrontFace& frontFace)
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

        
        VkPipelineVertexInputStateCreateInfo inputStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        inputStateInfo.vertexBindingDescriptionCount = 1;
        inputStateInfo.pVertexBindingDescriptions = &vBindingDescription;
        inputStateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vAttributeDescription.size());
        inputStateInfo.pVertexAttributeDescriptions = vAttributeDescription.data();
        
        
        VkPipelineInputAssemblyStateCreateInfo assemblyStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        assemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        assemblyStateInfo.primitiveRestartEnable = false;
        
        
        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = swapChainProps.extent.width;
        viewport.height = swapChainProps.extent.height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1.f;
        
        VkRect2D scissors;
        scissors.offset = { 0, 0 };
        scissors.extent = swapChainProps.extent;
        
        VkPipelineViewportStateCreateInfo viewportStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        viewportStateInfo.viewportCount = 1;
        viewportStateInfo.pViewports = &viewport;
        viewportStateInfo.scissorCount = 1;
        viewportStateInfo.pScissors = &scissors;
        
        
        VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        rasterizationStateInfo.depthClampEnable = false;
        rasterizationStateInfo.rasterizerDiscardEnable = false;
        rasterizationStateInfo.polygonMode = polygonMode;
        rasterizationStateInfo.cullMode = cullMode;
        rasterizationStateInfo.frontFace = frontFace;
        rasterizationStateInfo.depthBiasEnable = false;
        rasterizationStateInfo.depthBiasConstantFactor = 0;
        rasterizationStateInfo.depthBiasClamp = 0;
        rasterizationStateInfo.depthBiasSlopeFactor = 0;
        rasterizationStateInfo.lineWidth = 1.f;
        
        
        VkPipelineMultisampleStateCreateInfo multisampleStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        multisampleStateInfo.rasterizationSamples = getMaxUsableSampleCount();
        multisampleStateInfo.sampleShadingEnable = true;
        multisampleStateInfo.minSampleShading = 1.f;
        multisampleStateInfo.pSampleMask = nullptr;
        multisampleStateInfo.alphaToCoverageEnable = false;
        multisampleStateInfo.alphaToOneEnable = false;
        
        
        VkPipelineDepthStencilStateCreateInfo depthStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStateInfo.depthTestEnable = depthEnabled;
        depthStateInfo.depthWriteEnable = depthEnabled;
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
        layoutInfo.setLayoutCount = descriptor ? 1 : 0;
        layoutInfo.pSetLayouts = descriptor ? &descriptor->layout : nullptr;
        layoutInfo.pushConstantRangeCount = 0;
        layoutInfo.pPushConstantRanges = nullptr;
        
        VkPipelineLayout pipelineLayout;
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
        
        VkPipeline graphicsPipeline;
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) throw std::runtime_error("createGraphicsPipeline() failed!");
        return std::make_pair(graphicsPipeline, pipelineLayout);
    }

    void Context::generateDefaultVertexAndIndexBuffers(const VkDeviceSize &bufferSizeV, Buffer &vertexBuffer, const void* vertexData, const VkDeviceSize &bufferSizeI, Buffer &indexBuffer, const void* indexData)
    {
        Buffer stagingVertex, stagingIndex;
        
        createBuffer(bufferSizeV, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingVertex.buffer, stagingVertex.memory.memory);
        createBuffer(bufferSizeI, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIndex.buffer, stagingIndex.memory.memory);
        
        void* data;
        vkMapMemory(device, stagingVertex.memory.memory, 0, bufferSizeV, 0, &data);
            memcpy(data, vertexData, (size_t) bufferSizeV);
        vkUnmapMemory(device, stagingVertex.memory.memory);
        
        vkMapMemory(device, stagingIndex.memory.memory, 0, bufferSizeI, 0, &data);
            memcpy(data, indexData, (size_t) bufferSizeI);
        vkUnmapMemory(device, stagingIndex.memory.memory);
        
        createBuffer(bufferSizeV, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer.buffer, vertexBuffer.memory.memory);
        createBuffer(bufferSizeI, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer.buffer, indexBuffer.memory.memory);
        
        VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
        
            VkBufferCopy regions;
            regions.srcOffset = 0;
            regions.dstOffset = 0;
            regions.size = bufferSizeV;
            vkCmdCopyBuffer(cmdBuffer, stagingVertex.buffer, vertexBuffer.buffer, 1, &regions);
            
            VkBufferCopy regions2;
            regions2.srcOffset = 0;
            regions2.dstOffset = 0;
            regions2.size = bufferSizeI;
            vkCmdCopyBuffer(cmdBuffer, stagingIndex.buffer, indexBuffer.buffer, 1, &regions2);
        
        endSingleTimeCommands(cmdBuffer);
        
        freeBuffer(stagingVertex);
        freeBuffer(stagingIndex);
    }

    Texture Context::generateTexture(const std::string &textureName, uint32_t maxMipLevels)
    {
        Texture texture;
        
        int width, height, comp;
        stbi_uc* pixels = stbi_load(textureName.c_str(), &width, &height, &comp, STBI_rgb_alpha);
        texture.image.width = static_cast<uint32_t>(width); texture.image.height = static_cast<uint32_t>(height);
        if (maxMipLevels == 0)
            texture.mip = static_cast<uint32_t>( std::floor( std::log2( std::max(texture.image.width, texture.image.height) ) ) ) + 1;
        else texture.mip = std::max(static_cast<uint32_t>( std::floor( std::log2( std::max(texture.image.width, texture.image.height) ) ) ) + 1, maxMipLevels);
        VkDeviceSize imageSize = width * height * 4;
        
        if (!pixels) throw std::runtime_error("createImage() failed to load image!");
        
        Buffer stagingBuffer;
        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingBuffer.buffer, stagingBuffer.memory.memory);
        
        void* data;
        vkMapMemory(device, stagingBuffer.memory.memory, 0, imageSize, 0, &data);
            memcpy(data, pixels, imageSize);
        vkUnmapMemory(device, stagingBuffer.memory.memory);
        
        stbi_image_free(pixels);
        
        createImage(texture.image.width, texture.image.height, texture.mip, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image.image,texture.image.memory.memory);
        
        transitionImageLayout(texture.image.image, texture.mip, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer.buffer, texture.image.image, width, height);
        generateMipmaps(texture.image.image, VK_FORMAT_R8G8B8A8_SRGB, width, height, texture.mip);
        
        texture.image.view = createImageView(texture.image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, texture.mip);
        
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
        samplerInfo.maxLod = static_cast<float>(texture.mip);
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
        samplerInfo.unnormalizedCoordinates = false;
        if (vkCreateSampler(device, &samplerInfo, nullptr, &texture.sampler) != VK_SUCCESS) throw std::runtime_error("createImage(1) failed!");
        
        freeBuffer(stagingBuffer);
        
        return texture;
    }
    
#pragma mark -
#pragma mark Command buffer

    void Context::createCommandPool()
    {
       VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
       createInfo.queueFamilyIndex = graphicsQueueIndex.value();
       
       if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS)
           throw std::runtime_error("createCommandPool() failed!");
       
       createInfo.queueFamilyIndex = graphicsQueueIndex.value();
       createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
       
       if (vkCreateCommandPool(device, &createInfo, nullptr, &shortPool) != VK_SUCCESS)
           throw std::runtime_error("createCommandPool() failed!");
    }

    void Context::createCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocateInfo.commandPool = commandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = static_cast<uint32_t>(framebuffer.size());
        
        commandBuffers.resize(framebuffer.size());
        if (vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers.data()) != VK_SUCCESS) throw std::runtime_error("createCommandPool() failed!");
    }

    VkCommandBuffer Context::beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = shortPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void Context::endSingleTimeCommands(VkCommandBuffer& commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, shortPool, 1, &commandBuffer);
    }

    void Context::beginRecord()
    {
        for (size_t i = 0; i < commandBuffers.size(); ++i)
        {
            VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            beginInfo.pInheritanceInfo = nullptr;
            
            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) throw std::runtime_error("beginRecord() failed!");
            
            std::vector<VkClearValue> clearValues(2);
            clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
            clearValues[1].depthStencil = { 1.f, 0 };
            VkRenderPassBeginInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = framebuffer[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapChainProps.extent;
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();
            
            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        }
    }
    void Context::endRecord()
    {
        for (size_t i = 0; i < commandBuffers.size(); ++i)
        {
            vkCmdEndRenderPass(commandBuffers[i]);
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) throw std::runtime_error("endRecord() failed!");
        }
    }

#pragma mark -
#pragma mark Allocation

    void Context::freeTexture(Texture& texture)
    {
        freeImage(texture.image);
        vkDestroySampler(device, texture.sampler, nullptr);
    }

    void Context::freeImage(Image& image)
    {
        vkDestroyImage(device, image.image, nullptr);
        vkFreeMemory(device, image.memory.memory, nullptr);
        vkDestroyImageView(device, image.view, nullptr);
    }

    void Context::freeBuffer(Buffer& buffer)
    {
        vkDestroyBuffer(device, buffer.buffer, nullptr);
        vkFreeMemory(device, buffer.memory.memory, nullptr);
    }

    void Context::freeDescriptor(Descriptor& descriptor)
    {
        vkDestroyDescriptorSetLayout(device, descriptor.layout, nullptr);
        vkDestroyDescriptorPool(device, descriptor.pool, nullptr);
    }

#pragma mark -
#pragma mark Buffer
        
    uint32_t Context::findMemoryType(const uint32_t& typeFilter, const VkMemoryPropertyFlags& properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(GPU, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void Context::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
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
    
    void Context::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
        
        VkBufferCopy regions;
        regions.srcOffset = 0;
        regions.dstOffset = 0;
        regions.size = size;
        vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &regions);
        
        endSingleTimeCommands(cmdBuffer);
    }

#pragma mark -
#pragma mark Image

    void Context::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
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
    
    VkImageView Context::createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspect, uint32_t mipLevels)
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
    
    void Context::transitionImageLayout(VkImage image, uint32_t mipLevels, VkImageLayout oldLayout, VkImageLayout newLayout)
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
    
    void Context::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
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
    
    void Context::generateMipmaps(VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(GPU, format, &properties);
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

#pragma mark -
#pragma mark Supporting functions

    VkSampleCountFlagBits Context::getMaxUsableSampleCount()
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(GPU, &properties);
        
        VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
        /* if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
           if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT; */
#ifdef _WIN32
        if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
        if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
        if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
        if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
#else
        if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
#endif
        
        return VK_SAMPLE_COUNT_1_BIT;
    }

    VkFormat Context::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(GPU, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) return format;
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) return format;
        }
        throw std::runtime_error("failed to find supported format!");
    }
    
    VkFormat Context::findDepthFormat() { return findSupportedFormat( { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                                                                        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT ); }

#pragma mark -
#pragma mark Draw

    uint32_t Context::beginDraw()
    {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
               
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) throw exceptions::VK_ERROR_OUT_OF_DATE_KHR();
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) throw std::runtime_error("vkAcquireNextImageKHR() failed!");

        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
           vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];
        
        return imageIndex;
    }
    void Context::endDraw(const uint32_t& imageIndex)
    {
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
        
        VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
        
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) throw exceptions::VK_ERROR_OUT_OF_DATE_KHR();
        else if (result != VK_SUCCESS) throw std::runtime_error("vkQueuePresentKHR() failed!");
        
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

}
