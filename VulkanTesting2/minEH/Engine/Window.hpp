//
//  Window.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 14.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Window_hpp
#define Window_hpp

#include <iostream>
#include <list>

#include "Config.hpp"
#include "Event.hpp"

namespace mh
{
    struct WindowSize { unsigned int width, height; };
    struct Window
    {
#if defined(MINEH_WINDOW_API_GLFW)
        GLFWwindow* window = nullptr;
        static float hdpi;
#endif
        std::list<Event> events;
        
        Window();
        Window(unsigned int width, unsigned int height, const std::string& caption = "minEH Application");
        ~Window();
        
        void create(unsigned int width, unsigned int height, const std::string& caption = "minEH Application");
        bool isOpen();
        
        bool waitEvent(Event& event);
        bool pollEvent(Event& event);
        void pushEvent(const Event& event);
        
        WindowSize getSize();
        
        void setSize(unsigned int width, unsigned int height);
        void setPosition(unsigned int x, unsigned int y);
        void setCursorMode(const Mouse::CursorMode& cursor);
        
        Mouse::CursorMode getCursorMode();
        
#if defined(MINEH_WINDOW_API_GLFW)
        static void callback_GLFWmousebuttonfun(GLFWwindow *, int, int, int);
        static void callback_GLFWcursorposfun(GLFWwindow *, double, double);
        static void callback_GLFWcursorenterfun(GLFWwindow *, int);
        static void callback_GLFWscrollfun(GLFWwindow *, double, double);
        static void callback_GLFWkeyfun(GLFWwindow *, int, int, int, int);
        static void callback_GLFWcharfun(GLFWwindow *, unsigned int);
        
        static void callback_GLFWwindowposfun(GLFWwindow *, int, int);
        static void callback_GLFWwindowsizefun(GLFWwindow *, int, int);
        static void callback_GLFWwindowclosefun(GLFWwindow *);
        static void callback_GLFWwindowrefreshfun(GLFWwindow *);
        static void callback_GLFWwindowfocusfun(GLFWwindow *, int);
        static void callback_GLFWwindowmaximizefun(GLFWwindow *, int);
        static void callback_GLFWframebuffersizefun(GLFWwindow *, int, int);
        static void callback_GLFWwindowcontentscalefun(GLFWwindow *, float, float);
#if defined(__APPLE__) && defined(TARGET_OS_OSX)
        static void callback_GLFWwindowocclusionfun(GLFWwindow *, int);
#else
        static void callback_GLFWwindowiconifyfun(GLFWwindow *, int);
#endif
#endif
    };
}

#endif /* Window_hpp */
