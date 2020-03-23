//
//  Engine.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 14.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Engine.hpp"

namespace mh
{
#pragma mark -
#pragma mark Entity

    void Entity::Update(const float& elapsedTime) { }
    void Entity::OnEvent(Event& event) { }
    void Entity::Draw() { }

#pragma mark -
#pragma mark Engine

    Engine::Engine()
    {
#if defined(MINEH_WINDOW_API_GLFW)
        glfwInit();
#endif
        window.create(800, 600);
        context.create(window);
        cameraView.init(context);
        
        cameras.emplace_back(cameraView);
        activeCamera = &cameras.back();
        activeCamera->init();
        
        cameraController.init(window, *activeCamera);
        
        
        // TODO: sprite must be a rect, I think, so vertices and indices should not be covered
        // TODO: texture collector (=> texture descriptor?)
        // TODO: pipeline and shader collector
        // TODO: scene - collection of 3D objects, which may form parent/child relationship
        
        
        fullscreen = new Mesh2D(context);
        fullscreen->model.vertices = {
            { { -1.f,  -1.f }, { 0.0f,  0.0f } },
            { {  1.f,  -1.f }, { 1.0f,  0.0f } },
            { { -1.f,   1.f }, { 0.0f,  1.0f } },
            { {  1.f,   1.f }, { 1.0f,  1.0f } } };
        fullscreen->model.indices = { 0, 1, 2,   1, 3, 2 };
        fullscreen->loadTexture("Data/Textures/80203078_p0.jpg");
        {
            float aspect = (float)fullscreen->texture.image.width/fullscreen->texture.image.height;
            if (aspect < context.swapChainProps.aspect)
                fullscreen->model.scale = { context.swapChainProps.aspect, context.swapChainProps.aspect/aspect };
            else fullscreen->model.scale = { aspect, 1.f };
        }
        fullscreen->init(); drawable.push_back(fullscreen);
        
        
        Mesh1D* mesh1d = new Mesh1D(context);
        mesh1d->model.vertices = {
            { {  0.9f,  0.9f }, { 0.0f,  0.0f } },
            { {   1.f,  0.9f }, { 1.0f,  0.0f } },
            { {  0.9f,   1.f }, { 0.0f,  1.0f } },
            { {   1.f,   1.f }, { 1.0f,  1.0f } } };
        mesh1d->model.indices = { 0, 1, 2,   1, 3, 2 };
        mesh1d->loadTexture("Data/Textures/80203078_p0.jpg");
        mesh1d->init(); drawable.push_back(mesh1d);
        
        
        Mesh3D* mesh;/* = new Mesh3D(context, cameraView);
        mesh->loadModel("Data/Models/chalet.obj");
        mesh->loadTexture("Data/Textures/chalet.jpg");
        mesh->model.rotation.x = glm::radians(-90.f);
        mesh->model.position = { 5.f, 0.f, -5.f };
        mesh->init(); drawable.push_back(mesh);*/
        
        
        mesh = new Mesh3D(context, cameraView);
        mesh->loadModel("Data/Models/GothLoli SF2/untitled.obj");
        mesh->loadTexture("Data/Models/GothLoli SF2/gothloli_d.jpg");
        mesh->model.scale = { 0.15f, 0.15f, 0.15f };
        mesh->init(); drawable.push_back(mesh);
        
        
        mesh = new Mesh3D(context, cameraView);
        mesh->model.vertices = {
            { { -0.5f,  -0.5f,   -1.f }, { 0.0f,  1.0f } },
            { {  0.5f,  -0.5f,   -1.f }, { 1.0f,  1.0f } },
            { { -0.5f,   0.5f,   -1.f }, { 0.0f,  0.0f } },
            { {  0.5f,   0.5f,   -1.f }, { 1.0f,  0.0f } } };
        mesh->model.indices = { 0, 1, 2,   1, 3, 2 }; // counter clockwise
        mesh->loadTexture("Data/Textures/texture.jpg");
        mesh->model.scale = { 2.f, 1.f, 1.f };
        mesh->model.position = { 2.f, 0.f, -2.f };
        mesh->init(); drawable.push_back(mesh);
        
        
        /*mesh2d = new Mesh2D(context);
        mesh2d->model.indices = { 0, 1, 2,   1, 3, 2 };
        mesh2d->loadTexture("Data/Textures/FUutq0IDAG8.jpg");
        {
            float spriteHeight = 0.5f, spriteWidth = (float)mesh2d->texture.image.width/mesh2d->texture.image.height * spriteHeight;
            float spriteX = -0.9f, spriteY = -0.9f;
            mesh2d->model.vertices = {
               { {               spriteX,                 spriteY },  { 0.0f,  0.0f } },
               { { spriteX + spriteWidth,                 spriteY },  { 1.0f,  0.0f } },
               { {               spriteX,  spriteY + spriteHeight },  { 0.0f,  1.0f } },
               { { spriteX + spriteWidth,  spriteY + spriteHeight },  { 1.0f,  1.0f } } };
        }
        mesh2d->init(); drawable.push_back(mesh2d);*/
        
        
        context.beginRecord();
        for (auto& e : drawable)
            for (uint32_t i = 0; i < context.commandBuffers.size(); ++i)
                e->onRecord(context.commandBuffers[i], i);
        context.endRecord();
    }
    
    Engine::~Engine()
    {
        cameraView.cleanup();
        for (auto& e : drawable) delete e;
        context.destroy();
#if defined(MINEH_WINDOW_API_GLFW)
        glfwTerminate();
#endif
    }
    
    void Engine::Update(const float& elapsedTime)
    {
        // for (auto& e : drawable) e.Update(elapsedTime);
    }
    
    void Engine::OnEvent(Event& event)
    {
        // for (auto& e : drawable) e.Event(event);
    }
    
    void Engine::Draw()
    {
        // for (auto& e : drawable) e.Draw();
    }
    
    void Engine::Run()
    {
        while (window.isOpen())
        {
            while (window.pollEvent(event))
            {
                cameraController.OnEvent(event);
                switch (event.type)
                {
                    case Event::Type::MousePressed: cout << "Event::Type::MousePressed: " << (int)event.data.mouse.button.button << " at " << event.data.mouse.button.x << " " << event.data.mouse.button.y << endl; break;
                    case Event::Type::MouseReleased: cout << "Event::Type::MouseReleased: " << (int)event.data.mouse.button.button << " at " << event.data.mouse.button.x << " " << event.data.mouse.button.y << endl; break;
                    case Event::Type::Resized: cout << "Event::Type::Resized: " << event.data.size.width << " " << event.data.size.height << endl; break;
                    case Event::Type::Iconified: contextIconified = event.data.integer; break;
                    case Event::Type::Closed: cout << "Event::Type::Closed" << endl; break;
                    default: break;
                }
            }
            while (contextIconified)
            {
                window.waitEvent(event);
                if (event.type == Event::Type::Iconified)
                    contextIconified = event.data.integer;
            }
            
            float elapsedTime = clock.restart();
            for (auto& d : drawable) d->update(elapsedTime);
            cameraController.Update(elapsedTime);
            
            try
            {
                uint32_t imageIndex = context.beginDraw();
                
                for (auto& d : drawable) d->updateUniformBuffers(imageIndex);
                activeCamera->updateUniformBuffers(imageIndex);
                
                context.endDraw(imageIndex);
            }
            catch (exceptions::VK_ERROR_OUT_OF_DATE_KHR& e)
            {
                context.recreate();
                cameraView.recreate();
                activeCamera->recreate();
                for (auto& e : drawable) e->recreate();
                
                {
                    float aspect = (float)fullscreen->texture.image.width/fullscreen->texture.image.height;
                    if (aspect < context.swapChainProps.aspect)
                        fullscreen->setScale({ context.swapChainProps.aspect, context.swapChainProps.aspect/aspect });
                    else fullscreen->setScale({ aspect, 1.f });
                }
                /*{
                    float aspect = (float)fullscreen->texture.image.width/fullscreen->texture.image.height;
                    if (aspect < context.swapChainProps.aspect) fullscreen->setScale({ 1.f, context.swapChainProps.aspect/aspect });
                    else fullscreen->setScale({ aspect / context.swapChainProps.aspect, 1.f });
                }*/
                
                context.beginRecord();
                for (auto& e : drawable)
                    for (uint32_t i = 0; i < context.commandBuffers.size(); ++i)
                        e->onRecord(context.commandBuffers[i], i);
                context.endRecord();
            }
        }
        
        vkDeviceWaitIdle(context.device);
    }
}
