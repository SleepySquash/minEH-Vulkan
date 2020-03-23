//
//  Clock.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 19.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Clock_hpp
#define Clock_hpp

#include <iostream>
#include <chrono>

namespace mh
{
    struct Clock
    {
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
        
        Clock();
        float restart();
    };
}

#endif /* Clock_hpp */
