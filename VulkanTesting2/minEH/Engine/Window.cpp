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
        glfwSetWindowMaximizeCallback(window, callback_GLFWwindowmaximizefun);
        glfwSetFramebufferSizeCallback(window, callback_GLFWframebuffersizefun);
        glfwSetWindowContentScaleCallback(window, callback_GLFWwindowcontentscalefun);
#if defined(__APPLE__) && defined(TARGET_OS_OSX)
        glfwSetWindowOcclusionCallback(window, callback_GLFWwindowocclusionfun);
        if (glfwGetWindowAttrib(window, GLFW_OCCLUDED))
        {
            Event event;
            event.type = Event::Type::Iconified;
            event.data.integer = 1;
            pushEvent(event);
        }
#else
        glfwSetWindowIconifyCallback(window, callback_GLFWwindowiconifyfun);
#endif
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
        glfwSetWindowUserPointer(window, this);
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
#if defined(MINEH_WINDOW_API_GLFW)
        glfwSetWindowSize(window, width, height);
#endif
    }

    void Window::setPosition(unsigned int x, unsigned int y)
    {
#if defined(MINEH_WINDOW_API_GLFW)
        glfwSetWindowPos(window, x, y);
#endif
    }

    void Window::setCursorMode(const Mouse::CursorMode& cursor)
    {
#if defined(MINEH_WINDOW_API_GLFW)
        switch (cursor)
        {
            default: glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); break;
            case Mouse::CursorMode::Hidden: glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); break;
            case Mouse::CursorMode::Disabled: glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); break;
        }
#endif
    }

    Mouse::CursorMode Window::getCursorMode()
    {
#if defined(MINEH_WINDOW_API_GLFW)
        int cursor = glfwGetInputMode(window, GLFW_CURSOR);
        switch (cursor)
        {
            default: return Mouse::CursorMode::Normal; break;
            case GLFW_CURSOR_HIDDEN: return Mouse::CursorMode::Hidden; break;
            case GLFW_CURSOR_DISABLED: return Mouse::CursorMode::Disabled; break;
        }
#endif
    }

#if defined(MINEH_WINDOW_API_GLFW)
    void Window::callback_GLFWmousebuttonfun(GLFWwindow* window, int button, int action, int modifiers)
    {
        Event event;
        
        event.type = action ? Event::Type::MousePressed : Event::Type::MouseReleased;
        event.data.mouse.button.button = static_cast<Mouse::Button>(button);
        double x, y; glfwGetCursorPos(window, &x, &y);
        event.data.mouse.button.x = static_cast<int>(x * Window::hdpi);
        event.data.mouse.button.y = static_cast<int>(y * Window::hdpi);
        
        Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        win->pushEvent(event);
    }
    void Window::callback_GLFWcursorposfun(GLFWwindow* window, double x, double y)
    {
        Event event;
        
        event.type = Event::Type::MouseMove;
        event.data.mouse.move.x = static_cast<int>(x * Window::hdpi);
        event.data.mouse.move.y = static_cast<int>(y * Window::hdpi);
        
        Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        win->pushEvent(event);
    }
    void Window::callback_GLFWcursorenterfun(GLFWwindow *, int)
    {
        
    }
    void Window::callback_GLFWscrollfun(GLFWwindow *, double, double)
    {
        
    }
    void Window::callback_GLFWkeyfun(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Event event;
        
        switch (action)
        {
            default: event.type = Event::Type::keyReleased; break;
            case 1: event.type = Event::Type::KeyPressed; break;
            case 2: event.type = Event::Type::KeyRepeat; break;
        }
        switch (key)
        {
            default: event.data.key = Keyboard::Key::Unknown; break;
            case GLFW_KEY_SPACE: event.data.key = Keyboard::Key::Space; break;
            case GLFW_KEY_APOSTROPHE: event.data.key = Keyboard::Key::Apostrophe; break;
            case GLFW_KEY_COMMA: event.data.key = Keyboard::Key::Comma; break;
            case GLFW_KEY_MINUS: event.data.key = Keyboard::Key::Minus; break;
            case GLFW_KEY_PERIOD: event.data.key = Keyboard::Key::Period; break;
            case GLFW_KEY_SLASH: event.data.key = Keyboard::Key::Slash; break;
            case GLFW_KEY_0: event.data.key = Keyboard::Key::Num0; break;
            case GLFW_KEY_1: event.data.key = Keyboard::Key::Num1; break;
            case GLFW_KEY_2: event.data.key = Keyboard::Key::Num2; break;
            case GLFW_KEY_3: event.data.key = Keyboard::Key::Num3; break;
            case GLFW_KEY_4: event.data.key = Keyboard::Key::Num4; break;
            case GLFW_KEY_5: event.data.key = Keyboard::Key::Num5; break;
            case GLFW_KEY_6: event.data.key = Keyboard::Key::Num6; break;
            case GLFW_KEY_7: event.data.key = Keyboard::Key::Num7; break;
            case GLFW_KEY_8: event.data.key = Keyboard::Key::Num8; break;
            case GLFW_KEY_9: event.data.key = Keyboard::Key::Num9; break;
            case GLFW_KEY_SEMICOLON: event.data.key = Keyboard::Key::Semicon; break;
            case GLFW_KEY_EQUAL: event.data.key = Keyboard::Key::Equal; break;
            case GLFW_KEY_A: event.data.key = Keyboard::Key::A; break;
            case GLFW_KEY_B: event.data.key = Keyboard::Key::B; break;
            case GLFW_KEY_C: event.data.key = Keyboard::Key::C; break;
            case GLFW_KEY_D: event.data.key = Keyboard::Key::D; break;
            case GLFW_KEY_E: event.data.key = Keyboard::Key::E; break;
            case GLFW_KEY_F: event.data.key = Keyboard::Key::F; break;
            case GLFW_KEY_G: event.data.key = Keyboard::Key::G; break;
            case GLFW_KEY_H: event.data.key = Keyboard::Key::H; break;
            case GLFW_KEY_I: event.data.key = Keyboard::Key::I; break;
            case GLFW_KEY_J: event.data.key = Keyboard::Key::J; break;
            case GLFW_KEY_K: event.data.key = Keyboard::Key::K; break;
            case GLFW_KEY_L: event.data.key = Keyboard::Key::L; break;
            case GLFW_KEY_M: event.data.key = Keyboard::Key::M; break;
            case GLFW_KEY_N: event.data.key = Keyboard::Key::N; break;
            case GLFW_KEY_O: event.data.key = Keyboard::Key::O; break;
            case GLFW_KEY_P: event.data.key = Keyboard::Key::P; break;
            case GLFW_KEY_Q: event.data.key = Keyboard::Key::Q; break;
            case GLFW_KEY_R: event.data.key = Keyboard::Key::R; break;
            case GLFW_KEY_S: event.data.key = Keyboard::Key::S; break;
            case GLFW_KEY_T: event.data.key = Keyboard::Key::T; break;
            case GLFW_KEY_U: event.data.key = Keyboard::Key::U; break;
            case GLFW_KEY_V: event.data.key = Keyboard::Key::V; break;
            case GLFW_KEY_W: event.data.key = Keyboard::Key::W; break;
            case GLFW_KEY_X: event.data.key = Keyboard::Key::X; break;
            case GLFW_KEY_Y: event.data.key = Keyboard::Key::Y; break;
            case GLFW_KEY_Z: event.data.key = Keyboard::Key::Z; break;
            case GLFW_KEY_LEFT_BRACKET: event.data.key = Keyboard::Key::LBracket; break;
            case GLFW_KEY_BACKSLASH: event.data.key = Keyboard::Key::Backslash; break;
            case GLFW_KEY_RIGHT_BRACKET: event.data.key = Keyboard::Key::RBracket; break;
            case GLFW_KEY_GRAVE_ACCENT: event.data.key = Keyboard::Key::GraveAccent; break;
            case GLFW_KEY_WORLD_1: event.data.key = Keyboard::Key::World1; break;
            case GLFW_KEY_WORLD_2: event.data.key = Keyboard::Key::World2; break;
            case GLFW_KEY_ESCAPE: event.data.key = Keyboard::Key::Escape; break;
            case GLFW_KEY_ENTER: event.data.key = Keyboard::Key::Enter; break;
            case GLFW_KEY_TAB: event.data.key = Keyboard::Key::Tab; break;
            case GLFW_KEY_BACKSPACE: event.data.key = Keyboard::Key::Backspace; break;
            case GLFW_KEY_INSERT: event.data.key = Keyboard::Key::Insert; break;
            case GLFW_KEY_DELETE: event.data.key = Keyboard::Key::Delete; break;
            case GLFW_KEY_RIGHT: event.data.key = Keyboard::Key::Right; break;
            case GLFW_KEY_LEFT: event.data.key = Keyboard::Key::Left; break;
            case GLFW_KEY_DOWN: event.data.key = Keyboard::Key::Down; break;
            case GLFW_KEY_UP: event.data.key = Keyboard::Key::Up; break;
            case GLFW_KEY_PAGE_UP: event.data.key = Keyboard::Key::PageUp; break;
            case GLFW_KEY_PAGE_DOWN: event.data.key = Keyboard::Key::PageDown; break;
            case GLFW_KEY_HOME: event.data.key = Keyboard::Key::Home; break;
            case GLFW_KEY_END: event.data.key = Keyboard::Key::End; break;
            case GLFW_KEY_CAPS_LOCK: event.data.key = Keyboard::Key::CapsLock; break;
            case GLFW_KEY_SCROLL_LOCK: event.data.key = Keyboard::Key::ScrollLock; break;
            case GLFW_KEY_NUM_LOCK: event.data.key = Keyboard::Key::NumLock; break;
            case GLFW_KEY_PRINT_SCREEN: event.data.key = Keyboard::Key::PrintScreen; break;
            case GLFW_KEY_PAUSE: event.data.key = Keyboard::Key::Pause; break;
            case GLFW_KEY_F1: event.data.key = Keyboard::Key::F1; break;
            case GLFW_KEY_F2: event.data.key = Keyboard::Key::F2; break;
            case GLFW_KEY_F3: event.data.key = Keyboard::Key::F3; break;
            case GLFW_KEY_F4: event.data.key = Keyboard::Key::F4; break;
            case GLFW_KEY_F5: event.data.key = Keyboard::Key::F5; break;
            case GLFW_KEY_F6: event.data.key = Keyboard::Key::F6; break;
            case GLFW_KEY_F7: event.data.key = Keyboard::Key::F7; break;
            case GLFW_KEY_F8: event.data.key = Keyboard::Key::F8; break;
            case GLFW_KEY_F9: event.data.key = Keyboard::Key::F9; break;
            case GLFW_KEY_F10: event.data.key = Keyboard::Key::F10; break;
            case GLFW_KEY_F11: event.data.key = Keyboard::Key::F11; break;
            case GLFW_KEY_F12: event.data.key = Keyboard::Key::F12; break;
            case GLFW_KEY_KP_0: event.data.key = Keyboard::Key::Kp0; break;
            case GLFW_KEY_KP_1: event.data.key = Keyboard::Key::Kp1; break;
            case GLFW_KEY_KP_2: event.data.key = Keyboard::Key::Kp2; break;
            case GLFW_KEY_KP_3: event.data.key = Keyboard::Key::Kp3; break;
            case GLFW_KEY_KP_4: event.data.key = Keyboard::Key::Kp4; break;
            case GLFW_KEY_KP_5: event.data.key = Keyboard::Key::Kp5; break;
            case GLFW_KEY_KP_6: event.data.key = Keyboard::Key::Kp6; break;
            case GLFW_KEY_KP_7: event.data.key = Keyboard::Key::Kp7; break;
            case GLFW_KEY_KP_8: event.data.key = Keyboard::Key::Kp8; break;
            case GLFW_KEY_KP_9: event.data.key = Keyboard::Key::Kp9; break;
            case GLFW_KEY_KP_DECIMAL: event.data.key = Keyboard::Key::KpDecimal; break;
            case GLFW_KEY_KP_DIVIDE: event.data.key = Keyboard::Key::KpDivide; break;
            case GLFW_KEY_KP_MULTIPLY: event.data.key = Keyboard::Key::KpMultiply; break;
            case GLFW_KEY_KP_SUBTRACT: event.data.key = Keyboard::Key::KpSubtract; break;
            case GLFW_KEY_KP_ADD: event.data.key = Keyboard::Key::KpAdd; break;
            case GLFW_KEY_KP_ENTER: event.data.key = Keyboard::Key::KpEnter; break;
            case GLFW_KEY_KP_EQUAL: event.data.key = Keyboard::Key::KpEqual; break;
            case GLFW_KEY_LEFT_SHIFT: event.data.key = Keyboard::Key::LShift; break;
            case GLFW_KEY_LEFT_CONTROL: event.data.key = Keyboard::Key::LControl; break;
            case GLFW_KEY_LEFT_ALT: event.data.key = Keyboard::Key::LAlt; break;
            case GLFW_KEY_LEFT_SUPER: event.data.key = Keyboard::Key::LSuper; break;
            case GLFW_KEY_RIGHT_SHIFT: event.data.key = Keyboard::Key::RShift; break;
            case GLFW_KEY_RIGHT_CONTROL: event.data.key = Keyboard::Key::RControl; break;
            case GLFW_KEY_RIGHT_ALT: event.data.key = Keyboard::Key::RAlt; break;
            case GLFW_KEY_RIGHT_SUPER: event.data.key = Keyboard::Key::RSuper; break;
            case GLFW_KEY_MENU: event.data.key = Keyboard::Key::Menu; break;
        }
        Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        win->pushEvent(event);
    }
    void Window::callback_GLFWcharfun(GLFWwindow *, unsigned int)
    {
        
    }

    void Window::callback_GLFWwindowposfun(GLFWwindow *, int, int)
    {
        
    }
    void Window::callback_GLFWwindowsizefun(GLFWwindow* window, int width, int height)
    {
        /*Event event;
        
        event.type = Event::Type::Resized;
        event.data.size.width = static_cast<uint32_t>(width * hdpi);
        event.data.size.height = static_cast<uint32_t>(height * hdpi);
        
        Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        win->pushEvent(event);*/
        // TODO: make unique (windowsize and) framebuffersize event, so only the last is remain in the list of events
    }
    void Window::callback_GLFWwindowclosefun(GLFWwindow* window)
    {
        Event event;
        event.type = Event::Type::Closed;
        Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        win->pushEvent(event);
    }
    void Window::callback_GLFWwindowrefreshfun(GLFWwindow *)
    {
        
    }
    void Window::callback_GLFWwindowfocusfun(GLFWwindow *, int)
    {
        
    }
    void Window::callback_GLFWwindowmaximizefun(GLFWwindow *, int)
    {
        
    }
    void Window::callback_GLFWframebuffersizefun(GLFWwindow* window, int width, int height)
    {
        
    }
    void Window::callback_GLFWwindowcontentscalefun(GLFWwindow *, float, float)
    {
        
    }
#if defined(__APPLE__) && defined(TARGET_OS_OSX)
    void Window::callback_GLFWwindowocclusionfun(GLFWwindow* window, int occulded)
    {
        Event event;
        event.type = Event::Type::Iconified;
        event.data.integer = occulded;
        Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        win->pushEvent(event);
    }
#else
    void Window::callback_GLFWwindowiconifyfun(GLFWwindow* window, int iconify)
    {
        Event event;
        event.type = Event::Type::Iconified;
        event.data.integer = iconify;
        Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        win->pushEvent(event);
    }
#endif
#endif
}
