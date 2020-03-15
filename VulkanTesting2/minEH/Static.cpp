//
//  Static.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 12.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Static.hpp"

namespace mh
{
    const std::vector<const char*> mhs::validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const bool mhs::enableValidationLayers = true;
    const int mhs::MAX_FRAMES_IN_FLIGHT = 1;
}
