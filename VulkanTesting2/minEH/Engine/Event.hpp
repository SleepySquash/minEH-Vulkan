//
//  Event.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 15.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Event_hpp
#define Event_hpp

#include <iostream>
#include <fstream>

#pragma mark -
#pragma mark Events
    struct EventData_Resize { uint32_t width, height; };
    struct EventData_MouseMove { int x, y; };
    struct EventData_MouseButton { int x, y; unsigned char button; };
    struct Event
    {
        enum class Type
        {
            Unknown,
            Resize,
            MouseMove,
            MousePressed,
            MouseReleased
        }
        type;
        
        union
        {
            EventData_Resize size;
            union
            {
                EventData_MouseMove move;
                EventData_MouseButton button;
            }
            mouse;
        }
        data;
    };

#endif /* Event_hpp */
