//
//  _old1_vma.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 06.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef _old1_vma_hpp
#define _old1_vma_hpp

#include <stdio.h>

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VmaAllocation& bufferMemory)
{
    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
     
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsage;
    allocInfo.requiredFlags = properties;
    
    if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &bufferMemory, nullptr) != VK_SUCCESS) throw std::runtime_error("createBuffer() failed!");
}

void dedicateBufferOfVector(const void* contents, uint32_t bufferSize, VkBuffer& buffer, VmaAllocation& allocation, VkBufferUsageFlags usage)
{
    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferAllocation);
    
    void* data;
    vmaMapMemory(allocator, stagingBufferAllocation, &data);
        memcpy(data, contents, (size_t) bufferSize);
    vmaUnmapMemory(allocator, stagingBufferAllocation);
    
    createBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, allocation);
    copyBuffer(stagingBuffer, buffer, bufferSize);
    
    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAllocation);
}

void createVertexAndIndexBuffers()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size(),
                 bufferSize2 = sizeof(indices[0]) * indices.size();
    
    VkBuffer stagingBuffer, stagingBuffer2;
    VmaAllocation stagingBufferAllocation, stagingBufferAllocation2;
    
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, 0, stagingBuffer, stagingBufferAllocation);
    createBuffer(bufferSize2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, 0, stagingBuffer2, stagingBufferAllocation2);
    
    void* data;
    vmaMapMemory(allocator, stagingBufferAllocation, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    vmaUnmapMemory(allocator, stagingBufferAllocation);
    
    vmaMapMemory(allocator, stagingBufferAllocation2, &data);
        memcpy(data, indices.data(), (size_t) bufferSize2);
    vmaUnmapMemory(allocator, stagingBufferAllocation2);
    
    createBuffer(bufferSize2, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 0, indexBuffer, indexBufferAllocation);
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 0, vertexBuffer, vertexBufferAllocation);
    
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
    
    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAllocation);
    vmaDestroyBuffer(allocator, stagingBuffer2, stagingBufferAllocation2);
}

#endif /* _old1_vma_hpp */
