//
//  Drawable.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 18.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Drawable.hpp"

namespace mh
{
    Drawable::~Drawable() { }
    void Drawable::recreate() { }
    void Drawable::updateUniformBuffers(const uint32_t& i, const bool& force) { }
    void Drawable::onRecord(VkCommandBuffer& commandBuffer, uint32_t i) { }
    void Drawable::update(const float &elapsedTime) { }
}
