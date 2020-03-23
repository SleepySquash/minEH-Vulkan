//
//  Sprite.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 18.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Mesh2D.hpp"

#include <glm/gtx/matrix_transform_2d.hpp>
#include <stb_image.h>

namespace mh
{
    Mesh2D::Mesh2D(Context& context) { this->context = &context; }
    Mesh2D::~Mesh2D()
    {
        if (!context) return;
        vkDestroyPipeline(context->device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
        
        for (auto& b : ubo.buffers) context->freeBuffer(b);
        context->freeBuffer(vertexBuffer);
        context->freeBuffer(indexBuffer);
        context->freeTexture(texture);
        context->freeDescriptor(descriptor);
        
        vkDestroyShaderModule(context->device, vertexShader, nullptr);
        vkDestroyShaderModule(context->device, fragmentShader, nullptr);
    }
    void Mesh2D::init()
    {
        vertexShader = context->loadShader("Data/Shaders/spv/mesh2d.vert.spv");
        fragmentShader = context->loadShader("Data/Shaders/spv/mesh2d.frag.spv");
        
        createUniformBuffers();
        createDescriptorSetLayout();
        createDescriptors();
        createGraphicsPipeline();
        
        context->generateDefaultVertexAndIndexBuffers(sizeof(model.vertices[0]) * model.vertices.size(), vertexBuffer, model.vertices.data(),
                                                      sizeof(model.indices[0]) * model.indices.size(),   indexBuffer,  model.indices.data());
    }
    void Mesh2D::recreate()
    {
        vkDestroyPipeline(context->device, graphicsPipeline, nullptr);
        vkDestroyDescriptorPool(context->device, descriptor.pool, nullptr);
        vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
        
        
        recreateUniformBuffers();
        createDescriptors();
        createGraphicsPipeline();
    }
    
    void Mesh2D::recreateUniformBuffers()
    {
        int difference = static_cast<int>(context->swapChainProps.images) - static_cast<int>(ubo.buffers.size());
        if (difference > 0)
        {
            ubo.buffers.resize(context->swapChainProps.images);
            for (size_t i = ubo.buffers.size() - difference; i < ubo.buffers.size(); ++i)
                context->createBuffer(sizeof(CameraBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, ubo.buffers[i].buffer, ubo.buffers[i].memory.memory);
        }
        else if (difference < 0)
        {
            for (size_t i = ubo.buffers.size() + difference; i > ubo.buffers.size() + difference; --i)
                context->freeBuffer(ubo.buffers[i]);
            ubo.buffers.resize(context->swapChainProps.images);
        }
    }

    void Mesh2D::createUniformBuffers()
    {
        ubo.buffers.resize(context->swapChainProps.images);
        for (uint32_t i = 0; i < ubo.buffers.size(); ++i)
        {
            context->createBuffer(sizeof(ubo.model), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, ubo.buffers[i].buffer, ubo.buffers[i].memory.memory);
            updateUniformBuffers(i, true);
        }
    }

    void Mesh2D::createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboBinding;
        uboBinding.binding = 0;
        uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboBinding.descriptorCount = 1;
        uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboBinding.pImmutableSamplers = nullptr;
        
        VkDescriptorSetLayoutBinding samplerBinding;
        samplerBinding.binding = 1;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.descriptorCount = 1;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerBinding.pImmutableSamplers = nullptr;
        
        std::vector<VkDescriptorSetLayoutBinding> bindings = { uboBinding, samplerBinding };

        VkDescriptorSetLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        createInfo.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(context->device, &createInfo, nullptr, &descriptor.layout) != VK_SUCCESS) throw std::runtime_error("createDescriptorSetLayout() failed!");
    }
    void Mesh2D::createDescriptors()
    {
        /// createDescriptorPool
        
        std::vector<VkDescriptorPoolSize> size(2);
        size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        size[0].descriptorCount = context->swapChainProps.images;
        size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        size[1].descriptorCount = context->swapChainProps.images;

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
            std::vector<VkWriteDescriptorSet> writeSets(2);
            
            VkDescriptorBufferInfo bufferInfo;
            bufferInfo.buffer = ubo.buffers[i].buffer;
            bufferInfo.offset = ubo.buffers[i].offset;
            bufferInfo.range = sizeof(ubo.model);

            writeSets[0].sType = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeSets[0].dstSet = descriptor.sets[i];
            writeSets[0].dstBinding = 0;
            writeSets[0].dstArrayElement = 0;
            writeSets[0].descriptorCount = 1;
            writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeSets[0].pImageInfo = nullptr;
            writeSets[0].pBufferInfo = &bufferInfo;
            writeSets[0].pTexelBufferView = nullptr;

            VkDescriptorImageInfo imageInfo;
            imageInfo.sampler = texture.sampler;
            imageInfo.imageView = texture.image.view;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            writeSets[1].sType = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeSets[1].dstSet = descriptor.sets[i];
            writeSets[1].dstBinding = 1;
            writeSets[1].dstArrayElement = 0;
            writeSets[1].descriptorCount = 1;
            writeSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeSets[1].pImageInfo = &imageInfo;
            writeSets[1].pBufferInfo = nullptr;
            writeSets[1].pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(context->device, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
        }
    }
    
    void Mesh2D::createGraphicsPipeline()
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
    
    void Mesh2D::onRecord(VkCommandBuffer& commandBuffer, uint32_t i)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptor.sets[i], 0, nullptr);
        
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
    }

    void Mesh2D::updateUniformBuffers(const uint32_t& i, const bool& force)
    {
        if (ubo.dirty || force)
        {
            ubo.model = glm::mat4(1.f);
            ubo.model = glm::scale(ubo.model, glm::vec3(1.f/context->swapChainProps.aspect, 1.f, 1.f));
            
            ubo.model = glm::translate(ubo.model, glm::vec3(model.position, 0.f));
            ubo.model = glm::rotate(ubo.model, model.rotation.x, glm::vec3(0.f, 0.f, 1.f));
            ubo.model = glm::scale(ubo.model, glm::vec3(model.scale, 1.f));
            
            void* data;
            vkMapMemory(context->device, ubo.buffers[i].memory.memory, 0, sizeof(ubo.model), 0, &data);
                memcpy(data, &ubo.model, sizeof(ubo.model));
            vkUnmapMemory(context->device, ubo.buffers[i].memory.memory);
            
            if (ubo.dirty) --ubo.dirty;
        }
    }

    void Mesh2D::dirty() { ubo.dirty = static_cast<uint32_t>(context->commandBuffers.size()); }
    void Mesh2D::update(const float &elapsedTime)
    {
        model.rotation.x += glm::radians(20.f) * elapsedTime; dirty();
    }

    void Mesh2D::loadTexture(const std::string& textureName) { texture = context->generateTexture(textureName, maxMipLevels); }

    void Mesh2D::setPosition(const glm::vec2& pos) { model.position = pos; dirty(); }
    void Mesh2D::setRotation(const float& rot) { model.rotation.x = rot; dirty(); }
    void Mesh2D::setScale(const glm::vec2& scl) { model.scale = scl; dirty(); }
    const glm::vec2& Mesh2D::getPosition() { return model.position; }
    const float& Mesh2D::getRotation() { return model.rotation.x; }
    const glm::vec2& Mesh2D::getScale() { return model.scale; }
}
