//
//  Static.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 12.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Static_hpp
#define Static_hpp

#include <vector>

namespace mh
{
    struct mhs
    {
        static const std::vector<const char*> validationLayers;
        static const bool enableValidationLayers;
        static const int MAX_FRAMES_IN_FLIGHT;
    };
}
using mh::mhs;

#endif /* Static_hpp */
