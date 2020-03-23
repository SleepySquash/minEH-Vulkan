//
//  Engine.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 14.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Engine_hpp
#define Engine_hpp

#include <iostream>
using std::cout;
using std::endl;

#include <exception>
#include <optional>
#include <fstream>

#include <vector>
#include <map>
#include <unordered_map>

#include "Clock.hpp"

#include "Context.hpp"
#include "Drawable/Camera.hpp"
#include "Drawable/CameraController.hpp"
#include "Drawable/Mesh1D.hpp"
#include "Drawable/Mesh2D.hpp"
#include "Drawable/Mesh3D.hpp"

namespace mh
{
#pragma mark -
#pragma mark Entity

    struct Entity
    {
        Context* context;
        
        virtual void Update(const float& elapsedTime);
        virtual void OnEvent(Event& event);
        virtual void Draw();
    };

#pragma mark -
#pragma mark Engine

    struct Engine
    {
        Window window;
        Context context;
        bool contextIconified;
        Clock clock;
        Event event;
        
        CameraView cameraView;
        FreeCameraController cameraController;
        Camera* activeCamera = nullptr;
        
        Mesh2D* fullscreen = nullptr;
        
        std::vector<Drawable*> drawable;
        std::vector<Camera> cameras;
        
    public:
        Engine();
        ~Engine();
        
        void Update(const float& elapsedTime);
        void OnEvent(Event& event);
        void Draw();
        
        void Run();
    };
}

#endif /* Engine_hpp */
