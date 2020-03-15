//
//  Mesh2D.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 11.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Mesh2D.hpp"
#include "Vulkan.hpp"

#include <stb_image.h>

namespace mh_obsolete
{
    void Mesh2D::init()
    {
        vertexShader = mh_obsolete::loadShader(device, "Data/Shaders/spv/rect2d.vert.spv");
        fragmentShader = mh_obsolete::loadShader(device, "Data/Shaders/spv/rect2d.frag.spv");
        
        createTexture();
        
        createDescriptorSetLayout();
        createDescriptorPool();
        createDescriptorSets();
        
        createGraphicsPipeline();
        createVertexAndIndexBuffers();
    }

    void Mesh2D::draw(VkCommandBuffer& commandBuffer, const uint32_t& i)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
        
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    }
    
    void Mesh2D::cleanupSwapchain()
    {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }
    void Mesh2D::recreateSwapchain()
    {
        cleanupSwapchain();
        
        createDescriptorPool();
        createDescriptorSets();
        
        createGraphicsPipeline();
    }

    void Mesh2D::cleanup()
    {
        cleanupSwapchain();
        
        vkDestroyDescriptorSetLayout(device, descriptorLayout, nullptr);
        
        vkDestroySampler(device, textureImageSampler, nullptr);
        vkDestroyImageView(device, textureImageView, nullptr);
        vkDestroyImage(device, textureImage, nullptr);
        vkFreeMemory(device, textureImageMemory, nullptr);
        
        vkDestroyShaderModule(device, vertexShader, nullptr);
        vkDestroyShaderModule(device, fragmentShader, nullptr);
        
        mh_obsolete::cleanBuffer(device, vertexBuffer, vertexBufferMemory);
        mh_obsolete::cleanBuffer(device, indexBuffer, indexBufferMemory);
    }






    void Mesh2D::createGraphicsPipeline()
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
        vBindingDescription.stride = sizeof(Vertex2D);
        vBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        std::vector<VkVertexInputAttributeDescription> vAttributeDescription(2);
        vAttributeDescription[0].binding = 0;
        vAttributeDescription[0].location = 0;
        vAttributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
        vAttributeDescription[0].offset = offsetof(Vertex2D, pos);

        vAttributeDescription[1].binding = 0;
        vAttributeDescription[1].location = 1;
        vAttributeDescription[1].format = VK_FORMAT_R32G32_SFLOAT;
        vAttributeDescription[1].offset = offsetof(Vertex2D, uv);
        
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
        rasterizationStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationStateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateInfo.depthBiasEnable = false;
        rasterizationStateInfo.depthBiasConstantFactor = 0;
        rasterizationStateInfo.depthBiasClamp = 0;
        rasterizationStateInfo.depthBiasSlopeFactor = 0;
        rasterizationStateInfo.lineWidth = 1.f;
        
        
        VkPipelineMultisampleStateCreateInfo multisampleStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        multisampleStateInfo.rasterizationSamples = mh_obsolete::getMaxUsableSampleCount(physicalDevice);
        multisampleStateInfo.sampleShadingEnable = true;
        multisampleStateInfo.minSampleShading = 1.f;
        multisampleStateInfo.pSampleMask = nullptr;
        multisampleStateInfo.alphaToCoverageEnable = false;
        multisampleStateInfo.alphaToOneEnable = false;
        
        
        VkPipelineDepthStencilStateCreateInfo depthStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStateInfo.depthTestEnable = false;
        depthStateInfo.depthWriteEnable = false;
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




    
    void Mesh2D::createVertexAndIndexBuffers()
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size(),
                     bufferSize2 = sizeof(indices[0]) * indices.size();
        
        VkBuffer stagingBuffer, stagingBuffer2;
        VkDeviceMemory stagingBufferAllocation, stagingBufferAllocation2;
        
        mh_obsolete::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferAllocation);
        mh_obsolete::createBuffer(device, physicalDevice, bufferSize2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer2, stagingBufferAllocation2);
        
        void* data;
        vkMapMemory(device, stagingBufferAllocation, 0, bufferSize2, 0, &data);
            memcpy(data, vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(device, stagingBufferAllocation);
        
        vkMapMemory(device, stagingBufferAllocation2, 0, bufferSize2, 0, &data);
            memcpy(data, indices.data(), (size_t) bufferSize2);
        vkUnmapMemory(device, stagingBufferAllocation2);
        
        mh_obsolete::createBuffer(device, physicalDevice, bufferSize2, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
        mh_obsolete::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
        
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
        
        mh_obsolete::cleanBuffer(device, stagingBuffer, stagingBufferAllocation);
        mh_obsolete::cleanBuffer(device, stagingBuffer2, stagingBufferAllocation2);
    }
    
    



    void Mesh2D::createTexture()
    {
        int width, height, comp;
        stbi_uc* pixels = stbi_load(TEXTURE_NAME.c_str(), &width, &height, &comp, STBI_rgb_alpha);
        textureMipLevels = static_cast<uint32_t>( std::floor( std::log2( std::max(width, height) ) ) ) + 1;
        VkDeviceSize imageSize = width * height * 4;
        
        if (!pixels) throw std::runtime_error("createImage() failed to load image!");
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        mh_obsolete::createBuffer(device, physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        
        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, imageSize);
        vkUnmapMemory(device, stagingBufferMemory);
        
        stbi_image_free(pixels);
        
        mh_obsolete::createImage(device, physicalDevice, width, height, textureMipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
        
        mh_obsolete::transitionImageLayout(device, shortlivedCommandPool, graphicsQueue, textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureMipLevels);
        mh_obsolete::copyBufferToImage(device, shortlivedCommandPool, graphicsQueue, stagingBuffer, textureImage, width, height);
        mh_obsolete::generateMipmaps(device, physicalDevice, shortlivedCommandPool, graphicsQueue, textureImage, VK_FORMAT_R8G8B8A8_SRGB, width, height, textureMipLevels);
        
        textureImageView = mh_obsolete::createImageView(device, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, textureMipLevels);
        
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




    void Mesh2D::createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding samplerBinding;
        samplerBinding.binding = 0;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.descriptorCount = 1;
        samplerBinding.pImmutableSamplers = nullptr;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        
        std::vector<VkDescriptorSetLayoutBinding> bindings = { samplerBinding };
        VkDescriptorSetLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        createInfo.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorLayout) != VK_SUCCESS) throw std::runtime_error("createDescriptorSetLayout() failed!");
    }
    
    void Mesh2D::createDescriptorPool()
    {
        std::vector<VkDescriptorPoolSize> size(1);
        size[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        size[0].descriptorCount = static_cast<uint32_t>(swapchainImages_size);
        
        VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        createInfo.maxSets = static_cast<uint32_t>(swapchainImages_size);
        createInfo.poolSizeCount = static_cast<uint32_t>(size.size());
        createInfo.pPoolSizes = size.data();
        if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) throw std::runtime_error("createDescriptorPool() failed!");
    }
    void Mesh2D::createDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(swapchainImages_size, descriptorLayout);
        
        VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(swapchainImages_size);
        allocInfo.pSetLayouts = layouts.data();
        descriptorSets.resize(swapchainImages_size);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) throw std::runtime_error("createDescriptorSets() failed!");
        
        for (size_t i = 0; i < descriptorSets.size(); ++i)
        {
            std::vector<VkWriteDescriptorSet> writeSets(1);
            
            VkDescriptorImageInfo imageInfo;
            imageInfo.sampler = textureImageSampler;
            imageInfo.imageView = textureImageView;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            
            writeSets[0].sType = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeSets[0].dstSet = descriptorSets[i];
            writeSets[0].dstBinding = 0;
            writeSets[0].dstArrayElement = 0;
            writeSets[0].descriptorCount = 1;
            writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeSets[0].pImageInfo = &imageInfo;
            writeSets[0].pBufferInfo = nullptr;
            writeSets[0].pTexelBufferView = nullptr;
            
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
        }
    }



    
    
    
    VkCommandBuffer Mesh2D::beginSingleTimeCommands()
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

    void Mesh2D::endSingleTimeCommands(VkCommandBuffer commandBuffer)
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
}
