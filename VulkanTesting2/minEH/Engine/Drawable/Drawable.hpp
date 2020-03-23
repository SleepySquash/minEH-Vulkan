//
//  Drawable.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 18.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Drawable_hpp
#define Drawable_hpp

#include <iostream>
#include <glm/fwd.hpp>

#include "../Context.hpp"
#include "Camera.hpp"

namespace mh
{
    struct Drawable
    {
        Context* context = nullptr;
        CameraView* cameraView = nullptr;
        
        virtual ~Drawable();
        virtual void recreate();
        virtual void updateUniformBuffers(const uint32_t& i, const bool& force = false);
        virtual void onRecord(VkCommandBuffer& commandBuffer, uint32_t i);
        virtual void update(const float &elapsedTime);
    };
}

#endif /* Drawable_hpp */
