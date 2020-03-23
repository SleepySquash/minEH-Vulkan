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

namespace mh
{
#pragma mark -
#pragma mark Keyboard

    struct Keyboard
    {
        enum class Key
        {
            Unknown, Q, W, E, R, T, Y, U, I, O, P, A, S, D, F, G, H, J, K, L, Z, X, C, V, B, N, M, Up, Down, Right, Left, LShift, RShift, LAlt, RAlt, LControl, RControl, Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Kp0, Kp1, Kp2, Kp3, Kp4, Kp5, Kp6, Kp7, Kp8, Kp9, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, Space, Enter, Escape, Apostrophe, Comma, Period, Minus, Slash, Semicon, Equal, LBracket, Backslash, RBracket, GraveAccent, World1, World2, Tab, Backspace, Insert, Delete, PageUp, PageDown, Home, End, CapsLock, ScrollLock, NumLock, PrintScreen, Pause, Menu, RSuper, LSuper, KpDecimal, KpDivide, KpMultiply, KpSubtract, KpAdd, KpEnter, KpEqual
        };
    };

#pragma mark -
#pragma mark Mouse

    struct Mouse
    {
        enum class Button
        {
            Left,
            Right,
            Middle
        };
        
        enum class CursorMode
        {
            Normal,
            Hidden,
            Disabled
        };
    };

#pragma mark -
#pragma mark Events
    struct EventData_Resize { uint32_t width, height; };
    struct EventData_MouseMove { int x, y; };
    struct EventData_MouseButton { int x, y; Mouse::Button button; };
    struct Event
    {
        enum class Type
        {
            Unknown,
            
            Resized,
            Iconified,
            Closed,
            
            MouseMove,
            MousePressed,
            MouseReleased,
            
            KeyPressed,
            keyReleased,
            KeyRepeat
        }
        type;
        
        union
        {
            int integer;
            EventData_Resize size;
            union
            {
                EventData_MouseMove move;
                EventData_MouseButton button;
            }
            mouse;
            Keyboard::Key key;
        }
        data;
    };
}

#endif /* Event_hpp */
