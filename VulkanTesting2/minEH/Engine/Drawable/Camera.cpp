//
//  Camera.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 18.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Camera.hpp"

namespace mh
{
    void CameraView::init(Context& context)
    {
        this->context = &context;
        createUniformBuffers();
    }
    void CameraView::recreate()
    {
        int difference = static_cast<int>(context->swapChainProps.images) - static_cast<int>(uniformBuffers.size());
        if (difference > 0)
        {
            uniformBuffers.resize(context->swapChainProps.images);
            for (size_t i = uniformBuffers.size() - difference; i < uniformBuffers.size(); ++i)
                context->createBuffer(sizeof(CameraBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uniformBuffers[i].buffer, uniformBuffers[i].memory.memory);
        }
        else if (difference < 0)
        {
            for (size_t i = uniformBuffers.size() + difference; i > uniformBuffers.size() + difference; --i)
                context->freeBuffer(uniformBuffers[i]);
            uniformBuffers.resize(context->swapChainProps.images);
        }
    }
    void CameraView::cleanup() { if (context) for (auto& b : uniformBuffers) context->freeBuffer(b); }
    void CameraView::createUniformBuffers()
    {
        uniformBuffers.resize(context->swapChainProps.images);
        for (size_t i = 0; i < uniformBuffers.size(); ++i)
            context->createBuffer(sizeof(CameraBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uniformBuffers[i].buffer, uniformBuffers[i].memory.memory);
    }


    Camera::Camera(CameraView& view, const float& fov, const float& near, const float& far) : view(view), fov(fov), near(near), far(far) { }
    void Camera::init() { cbo.view = glm::lookAt(position, position - direction, up); recreate(); }
    void Camera::recreate()
    {
        cbo.proj = glm::perspective(fov, view.context->swapChainProps.extent.width / (float)view.context->swapChainProps.extent.height, near, far);
        cbo.proj[1][1] *= -1;
        for (uint32_t i = 0; i < view.context->commandBuffers.size(); ++i) updateUniformBuffers(i, true);
    }
    void Camera::update()
    {
        cbo.view = glm::lookAt(position, position - direction, up);
        dirtyBuffers = static_cast<uint32_t>(view.context->commandBuffers.size());
    }
    void Camera::updateUniformBuffers(const uint32_t& i, const bool& force)
    {
        if (force || dirtyBuffers)
        {
            void* data;
            vkMapMemory(view.context->device, view.uniformBuffers[i].memory.memory, 0, sizeof(CameraBufferObject), 0, &data);
                memcpy(data, &cbo, sizeof(CameraBufferObject));
            vkUnmapMemory(view.context->device, view.uniformBuffers[i].memory.memory);
            
            if (dirtyBuffers) --dirtyBuffers;
        }
    }
}
