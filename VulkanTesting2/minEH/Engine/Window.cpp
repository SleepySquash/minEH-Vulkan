//
//  Window.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 14.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Window.hpp"

namespace mh
{
#if defined(MINEH_WINDOW_API_GLFW)
    float Window::hdpi = 1.f;
#endif

    Window::Window() { }
    Window::Window(unsigned int width, unsigned int height, const std::string& caption) { create(width, height, caption); }
    Window::~Window()
    {
#if defined(MINEH_WINDOW_API_GLFW)
        if (window) glfwDestroyWindow(window);
#endif
    }
    
    void Window::create(unsigned int width, unsigned int height, const std::string& caption)
    {
#if defined(MINEH_WINDOW_API_GLFW)
        if (window) glfwDestroyWindow(window);
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(width, height, caption.c_str(), nullptr, nullptr);
        
        int wwidth, wheight, fwidth, fheight;
        glfwGetWindowSize(window, &wwidth, &wheight);
        glfwGetFramebufferSize(window, &fwidth, &fheight);
        hdpi = (float)fwidth / wwidth;
        
    #if !defined(MINEH_WINDOW_MULTIPLE_WINDOWS)
        glfwSetWindowUserPointer(window, this);
    #endif
        
        glfwSetKeyCallback(window, callback_GLFWkeyfun);
        glfwSetCharCallback(window, callback_GLFWcharfun);
        glfwSetMouseButtonCallback(window, callback_GLFWmousebuttonfun);
        glfwSetCursorPosCallback(window, callback_GLFWcursorposfun);
        glfwSetCursorEnterCallback(window, callback_GLFWcursorenterfun);
        glfwSetScrollCallback(window, callback_GLFWscrollfun);
        
        glfwSetWindowPosCallback(window, callback_GLFWwindowposfun);
        glfwSetWindowSizeCallback(window, callback_GLFWwindowsizefun);
        glfwSetWindowCloseCallback(window, callback_GLFWwindowclosefun);
        glfwSetWindowRefreshCallback(window, callback_GLFWwindowrefreshfun);
        glfwSetWindowFocusCallback(window, callback_GLFWwindowfocusfun);
        glfwSetWindowIconifyCallback(window, callback_GLFWwindowiconifyfun);
        glfwSetWindowMaximizeCallback(window, callback_GLFWwindowmaximizefun);
        glfwSetFramebufferSizeCallback(window, callback_GLFWframebuffersizefun);
        glfwSetWindowContentScaleCallback(window, callback_GLFWwindowcontentscalefun);
        glfwSetWindowOcclusionCallback(window, callback_GLFWwindowocclusionfun);
#endif
    }

    bool Window::isOpen()
    {
#if defined(MINEH_WINDOW_API_GLFW)
        return !glfwWindowShouldClose(window);
#endif
    }

    bool Window::waitEvent(Event& event)
    {
#if defined(MINEH_WINDOW_API_GLFW)
    #if defined(MINEH_WINDOW_MULTIPLE_WINDOWS)
        glfwSetWindowUserPointer(window, this):
    #endif
        glfwWaitEvents();
#endif
        if (!events.empty())
        {
            event = events.front();
            events.pop_front();
            return true;
        }
        return false;
    }
    bool Window::pollEvent(Event& event)
    {
#if defined(MINEH_WINDOW_API_GLFW)
    #if defined(MINEH_WINDOW_MULTIPLE_WINDOWS)
        glfwSetWindowUserPointer(window, this):
    #endif
        glfwPollEvents();
#endif
        if (!events.empty())
        {
            event = events.front();
            events.pop_front();
            return true;
        }
        return false;
    }
    void Window::pushEvent(const Event &event) { events.emplace_back(event); }

    WindowSize Window::getSize()
    {
#if defined(MINEH_WINDOW_API_GLFW)
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        return { static_cast<unsigned int>(width), static_cast<unsigned int>(height) };
#endif
    }
    
    void Window::setSize(unsigned int width, unsigned int height)
    {
        
    }
    void Window::setPosition(unsigned int x, unsigned int y)
    {
        
    }

#if defined(MINEH_WINDOW_API_GLFW)
    void Window::callback_GLFWmousebuttonfun(GLFWwindow* window, int button, int action, int modifiers)
    {
        Event event;
        
        event.type = action ? Event::Type::MousePressed : Event::Type::MouseReleased;
        event.data.mouse.button.button = static_cast<unsigned char>(button);
        double x, y; glfwGetCursorPos(window, &x, &y);
        event.data.mouse.button.x = static_cast<int>(x * Window::hdpi);
        event.data.mouse.button.y = static_cast<int>(y * Window::hdpi);
        
        Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        win->pushEvent(event);
    }
    void Window::callback_GLFWcursorposfun(GLFWwindow *, double, double)
    {
        
    }
    void Window::callback_GLFWcursorenterfun(GLFWwindow *, int)
    {
        
    }
    void Window::callback_GLFWscrollfun(GLFWwindow *, double, double)
    {
        
    }
    void Window::callback_GLFWkeyfun(GLFWwindow *, int, int, int, int)
    {
        
    }
    void Window::callback_GLFWcharfun(GLFWwindow *, unsigned int)
    {
        
    }

    void Window::callback_GLFWwindowposfun(GLFWwindow *, int, int)
    {
        
    }
    void Window::callback_GLFWwindowsizefun(GLFWwindow *, int, int)
    {
        
    }
    void Window::callback_GLFWwindowclosefun(GLFWwindow *)
    {
        
    }
    void Window::callback_GLFWwindowrefreshfun(GLFWwindow *)
    {
        
    }
    void Window::callback_GLFWwindowfocusfun(GLFWwindow *, int)
    {
        
    }
    void Window::callback_GLFWwindowiconifyfun(GLFWwindow *, int)
    {
        
    }
    void Window::callback_GLFWwindowmaximizefun(GLFWwindow *, int)
    {
        
    }
    void Window::callback_GLFWframebuffersizefun(GLFWwindow *, int, int)
    {
        
    }
    void Window::callback_GLFWwindowcontentscalefun(GLFWwindow *, float, float)
    {
        
    }
    void Window::callback_GLFWwindowocclusionfun(GLFWwindow* window, int occulded)
    {
        
    }
#endif
}
