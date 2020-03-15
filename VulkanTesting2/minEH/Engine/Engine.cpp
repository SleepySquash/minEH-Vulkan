//
//  Engine.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 14.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Engine.hpp"
#include <stb_image.h>

namespace mh
{

#pragma mark -
#pragma mark Drawable

    Drawable::~Drawable() { }

    Sprite::Sprite(Context& context, const std::string& path) : textureName(path) { this->context = &context; }

    Sprite::~Sprite()
    {
        vkDestroyPipeline(context->device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
        
        context->freeBuffer(vertexBuffer);
        context->freeBuffer(indexBuffer);
        context->freeTexture(texture);
        context->freeDescriptor(descriptor);
        
        vkDestroyShaderModule(context->device, vertexShader, nullptr);
        vkDestroyShaderModule(context->device, fragmentShader, nullptr);
    }
    void Sprite::init()
    {
        vertexShader = context->loadShader("Data/Shaders/spv/rect2d.vert.spv");
        fragmentShader = context->loadShader("Data/Shaders/spv/rect2d.frag.spv");
        
        createTexture();
        
        createDescriptorSetLayout();
        createDescriptorPool();
        createDescriptorSets();
        
        createGraphicsPipeline();
        createVertexAndIndexBuffers();
    }
    void Sprite::recreate()
    {
        vkDestroyPipeline(context->device, graphicsPipeline, nullptr);
        vkDestroyDescriptorPool(context->device, descriptor.pool, nullptr);
        vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
        
        
        createDescriptorPool();
        createDescriptorSets();
        
        createGraphicsPipeline();
    }
    
    void Sprite::createTexture()
    {
        int width, height, comp;
        stbi_uc* pixels = stbi_load(textureName.c_str(), &width, &height, &comp, STBI_rgb_alpha);
        texture.image.width = static_cast<uint32_t>(width); texture.image.height = static_cast<uint32_t>(height);
        texture.mip = static_cast<uint32_t>( std::floor( std::log2( std::max(texture.image.width, texture.image.height) ) ) ) + 1;
        VkDeviceSize imageSize = width * height * 4;
        
        if (!pixels) throw std::runtime_error("createImage() failed to load image!");
        
        Buffer stagingBuffer;
        context->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.buffer, stagingBuffer.memory.memory);
        
        void* data;
        vkMapMemory(context->device, stagingBuffer.memory.memory, 0, imageSize, 0, &data);
            memcpy(data, pixels, imageSize);
        vkUnmapMemory(context->device, stagingBuffer.memory.memory);
        
        stbi_image_free(pixels);
        
        context->createImage(texture.image.width, texture.image.height, texture.mip, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image.image, texture.image.memory.memory);
        
        context->transitionImageLayout(texture.image.image, texture.mip, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        context->copyBufferToImage(stagingBuffer.buffer, texture.image.image, width, height);
        context->generateMipmaps(texture.image.image, VK_FORMAT_R8G8B8A8_SRGB, width, height, texture.mip);
        
        texture.image.view = context->createImageView(texture.image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, texture.mip);
        
        VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.mipLodBias = 0;
        samplerInfo.anisotropyEnable = context->anisotropyEnable;
        samplerInfo.maxAnisotropy = context->anisotropyEnable ? 16 : 0;
        samplerInfo.compareEnable = false;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.minLod = 0;
        samplerInfo.maxLod = static_cast<float>(texture.mip);
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
        samplerInfo.unnormalizedCoordinates = false;
        if (vkCreateSampler(context->device, &samplerInfo, nullptr, &texture.sampler) != VK_SUCCESS) throw std::runtime_error("createImage(1) failed!");
        
        context->freeBuffer(stagingBuffer);
    }
    
    void Sprite::createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding samplerBinding;
        samplerBinding.binding = 0;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.descriptorCount = 1;
        samplerBinding.pImmutableSamplers = nullptr;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        createInfo.bindingCount = 1;
        createInfo.pBindings = &samplerBinding;
        if (vkCreateDescriptorSetLayout(context->device, &createInfo, nullptr, &descriptor.layout) != VK_SUCCESS) throw std::runtime_error("createDescriptorSetLayout() failed!");
    }
    
    void Sprite::createDescriptorPool()
    {
        std::vector<VkDescriptorPoolSize> size(1);
        size[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        size[0].descriptorCount = context->swapChainProps.images;

        VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        createInfo.maxSets = context->swapChainProps.images;
        createInfo.poolSizeCount = static_cast<uint32_t>(size.size());
        createInfo.pPoolSizes = size.data();
        if (vkCreateDescriptorPool(context->device, &createInfo, nullptr, &descriptor.pool) != VK_SUCCESS) throw std::runtime_error("createDescriptorPool() failed!");
    }

    void Sprite::createDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(context->swapChainProps.images, descriptor.layout);

        VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.descriptorPool = descriptor.pool;
        allocInfo.descriptorSetCount = context->swapChainProps.images;
        allocInfo.pSetLayouts = layouts.data();
        descriptor.sets.resize(context->swapChainProps.images);
        if (vkAllocateDescriptorSets(context->device, &allocInfo, descriptor.sets.data()) != VK_SUCCESS) throw std::runtime_error("createDescriptorSets() failed!");

        for (size_t i = 0; i < descriptor.sets.size(); ++i)
        {
            std::vector<VkWriteDescriptorSet> writeSets(1);

            VkDescriptorImageInfo imageInfo;
            imageInfo.sampler = texture.sampler;
            imageInfo.imageView = texture.image.view;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            writeSets[0].sType = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeSets[0].dstSet = descriptor.sets[i];
            writeSets[0].dstBinding = 0;
            writeSets[0].dstArrayElement = 0;
            writeSets[0].descriptorCount = 1;
            writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeSets[0].pImageInfo = &imageInfo;
            writeSets[0].pBufferInfo = nullptr;
            writeSets[0].pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(context->device, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
        }
    }
    
    void Sprite::createGraphicsPipeline()
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
        
        
        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = context->swapChainProps.extent.width;
        viewport.height = context->swapChainProps.extent.height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1.f;
        
        VkRect2D scissors;
        scissors.offset = { 0, 0 };
        scissors.extent = context->swapChainProps.extent;
        
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
        multisampleStateInfo.rasterizationSamples = context->getMaxUsableSampleCount();
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
        layoutInfo.pSetLayouts = &descriptor.layout;
        layoutInfo.pushConstantRangeCount = 0;
        layoutInfo.pPushConstantRanges = nullptr;
        
        if (vkCreatePipelineLayout(context->device, &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
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
        createInfo.renderPass = context->renderPass;
        createInfo.subpass = 0;
        createInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        createInfo.basePipelineIndex = -1; // Optional
        
        if (vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) throw std::runtime_error("createGraphicsPipeline() failed!");
    }
    
    void Sprite::createVertexAndIndexBuffers()
    {
        VkDeviceSize bufferSizeV = sizeof(vertices[0]) * vertices.size(),
                     bufferSizeI = sizeof(indices[0]) * indices.size();
        
        Buffer stagingVertex, stagingIndex;
        
        context->createBuffer(bufferSizeV, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingVertex.buffer, stagingVertex.memory.memory);
        context->createBuffer(bufferSizeI, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIndex.buffer, stagingIndex.memory.memory);
        
        void* data;
        vkMapMemory(context->device, stagingVertex.memory.memory, 0, bufferSizeV, 0, &data);
            memcpy(data, vertices.data(), (size_t) bufferSizeV);
        vkUnmapMemory(context->device, stagingVertex.memory.memory);
        
        vkMapMemory(context->device, stagingIndex.memory.memory, 0, bufferSizeI, 0, &data);
            memcpy(data, indices.data(), (size_t) bufferSizeI);
        vkUnmapMemory(context->device, stagingIndex.memory.memory);
        
        context->createBuffer(bufferSizeV, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer.buffer, vertexBuffer.memory.memory);
        context->createBuffer(bufferSizeI, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer.buffer, indexBuffer.memory.memory);
        
        VkCommandBuffer cmdBuffer = context->beginSingleTimeCommands();
        
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
        
        context->endSingleTimeCommands(cmdBuffer);
        
        context->freeBuffer(stagingVertex);
        context->freeBuffer(stagingIndex);
    }
    
    void Sprite::onRecord(VkCommandBuffer& commandBuffer, uint32_t i)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptor.sets[i], 0, nullptr);
        
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    }


#pragma mark -
#pragma mark Events


#pragma mark -
#pragma mark Entity


#pragma mark -
#pragma mark Engine

}
