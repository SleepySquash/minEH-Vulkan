//
//  Clock.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 19.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Clock.hpp"

namespace mh
{
    Clock::Clock() { startTime = std::chrono::high_resolution_clock::now(); }
    float Clock::restart()
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        startTime = currentTime;
        
        return time;
    }
}
