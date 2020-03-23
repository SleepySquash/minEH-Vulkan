//
//  Mesh1D.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 23.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Mesh1D.hpp"
#include <stb_image.h>

namespace mh
{
    Mesh1D::Mesh1D(Context& context) { this->context = &context; }
    Mesh1D::~Mesh1D()
    {
        if (!context) return;
        vkDestroyPipeline(context->device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
        
        context->freeBuffer(vertexBuffer);
        context->freeBuffer(indexBuffer);
        context->freeTexture(texture);
        context->freeDescriptor(descriptor);
        
        vkDestroyShaderModule(context->device, vertexShader, nullptr);
        vkDestroyShaderModule(context->device, fragmentShader, nullptr);
    }
    void Mesh1D::init()
    {
        vertexShader = context->loadShader("Data/Shaders/spv/rect2d.vert.spv");
        fragmentShader = context->loadShader("Data/Shaders/spv/rect2d.frag.spv");
        
        createDescriptorSetLayout();
        createDescriptors();
        createGraphicsPipeline();
        
        context->generateDefaultVertexAndIndexBuffers(sizeof(model.vertices[0]) * model.vertices.size(), vertexBuffer, model.vertices.data(),
                                                      sizeof(model.indices[0]) * model.indices.size(),   indexBuffer,  model.indices.data());
    }
    void Mesh1D::recreate()
    {
        vkDestroyPipeline(context->device, graphicsPipeline, nullptr);
        vkDestroyDescriptorPool(context->device, descriptor.pool, nullptr);
        vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
        
        
        createDescriptors();
        createGraphicsPipeline();
    }
    
    void Mesh1D::createDescriptorSetLayout()
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
    void Mesh1D::createDescriptors()
    {
        /// createDescriptorPool
        
        std::vector<VkDescriptorPoolSize> size(1);
        size[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        size[0].descriptorCount = context->swapChainProps.images;

        VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        createInfo.maxSets = context->swapChainProps.images;
        createInfo.poolSizeCount = static_cast<uint32_t>(size.size());
        createInfo.pPoolSizes = size.data();
        if (vkCreateDescriptorPool(context->device, &createInfo, nullptr, &descriptor.pool) != VK_SUCCESS) throw std::runtime_error("createDescriptorPool() failed!");
        
        /// createDescriptorSets
        
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
    
    void Mesh1D::createGraphicsPipeline()
    {
        VkVertexInputBindingDescription vBindingDescription = {};
        vBindingDescription.binding = 0;
        vBindingDescription.stride = sizeof(Vertex<glm::vec2>);
        vBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::vector<VkVertexInputAttributeDescription> vAttributeDescription(2);
        vAttributeDescription[0].binding = 0;
        vAttributeDescription[0].location = 0;
        vAttributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
        vAttributeDescription[0].offset = offsetof(Vertex<glm::vec2>, pos);

        vAttributeDescription[1].binding = 0;
        vAttributeDescription[1].location = 1;
        vAttributeDescription[1].format = VK_FORMAT_R32G32_SFLOAT;
        vAttributeDescription[1].offset = offsetof(Vertex<glm::vec2>, uv);
        
        auto [graphicsPipeline, pipelineLayout] = context->generateDefaultPipeline(vertexShader, fragmentShader, vAttributeDescription, vBindingDescription, &descriptor, false, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
        this->graphicsPipeline = graphicsPipeline;
        this->pipelineLayout = pipelineLayout;
    }
    
    void Mesh1D::onRecord(VkCommandBuffer& commandBuffer, uint32_t i)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptor.sets[i], 0, nullptr);
        
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
    }

    void Mesh1D::loadTexture(const std::string& textureName) { texture = context->generateTexture(textureName, maxMipLevels); }
}
