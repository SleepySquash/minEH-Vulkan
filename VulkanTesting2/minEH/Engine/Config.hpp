//
//  Config.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 14.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Config_hpp
#define Config_hpp


#ifndef MINEH_WINDOW_API_CUSTOM
    #if defined(__APPLE__)
        #include <TargetConditionals.h>
        #if defined(TARGET_OS_OSX)
            #define MINEH_WINDOW_API_GLFW   // TODO: MINEH_WINDOW_API_METAL
        #else
            #define MINEH_WINDOW_API_METAL
        #endif
    #elif defined(__ANDROID__)
        #define MINEH_WINDOW_API_JNI
    #elif defined(_WIN32)
        #define MINEH_WINDOW_API_GLFW   // TODO: MINEH_WINDOW_API_WIN32
    #elif defined(unix) || defined(__unix__) || defined(__unix)
        #define MINEH_WINDOW_API_GLFW   // TODO: MINEH_WINDOW_API_XLIB
    #endif
#endif


#if defined(MINEH_WINDOW_API_GLFW)
    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>
#elif defined(MINEH_WINDOW_API_METAL)
    // TODO
#elif defined(MINEH_WINDOW_API_JNI)
    // TODO
#elif defined(MINEH_WINDOW_API_WIN32)
    // TODO
#elif defined(MINEH_WINDOW_API_XLIB)
    // TODO
#endif


#endif /* Config_hpp */
