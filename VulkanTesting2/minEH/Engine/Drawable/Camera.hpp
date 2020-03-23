//
//  Camera.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 18.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Camera_hpp
#define Camera_hpp

#include <iostream>
#include <vector>

#include "../Context.hpp"

namespace mh
{
    struct CameraView
    {
        Context* context = nullptr;
        std::vector<Buffer> uniformBuffers;
        
        void init(Context& context);
        void recreate();
        void cleanup();
        void createUniformBuffers();
    };
    
    struct Camera
    {
        CameraView& view;
        CameraBufferObject cbo;
        uint32_t dirtyBuffers = 0;
        
        glm::vec3 position = glm::vec3(0.f, 0.f, 1.f),
                  direction = glm::vec3(0.f, 0.f, 1.f),
                  up = glm::vec3(0.f, 1.f, 0.f);
        float fov, near, far;
        
        Camera(CameraView& view, const float& fov = glm::radians(60.f), const float& near = 0.01f, const float& far = 100.f);
        void init();
        void recreate();
        void update();
        void updateUniformBuffers(const uint32_t& i, const bool& force = false);
    };
}

#endif /* Camera_hpp */
