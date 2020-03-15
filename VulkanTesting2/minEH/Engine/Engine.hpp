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

#include "Config.hpp"
#include <glm/glm.hpp>

#include "../Static.hpp"
#include "../Vertex.hpp"

#include "Config.hpp"
#include "Window.hpp"
#include "Context.hpp"
#include "ObjectContext.hpp"

namespace mh
{

#pragma mark -
#pragma mark Drawable
    struct Drawable
    {
        Context* context = nullptr;
        virtual ~Drawable();
        virtual void recreate() = 0;
        virtual void onRecord(VkCommandBuffer& commandBuffer, uint32_t i) = 0;
    };

    struct Sprite : Drawable
    {
        std::string textureName;
        
        std::vector<Vertex2D> vertices;
        std::vector<uint32_t> indices;
        
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;
        
        VkShaderModule vertexShader = VK_NULL_HANDLE,
                       fragmentShader = VK_NULL_HANDLE;
        
        Buffer vertexBuffer, indexBuffer;
        Descriptor descriptor;
        Texture texture;
        
        Sprite(Context& context, const std::string& path);
        ~Sprite();
        void init();
        void recreate() override;
        void createTexture();
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSets();
        void createGraphicsPipeline();
        void createVertexAndIndexBuffers();
        void onRecord(VkCommandBuffer& commandBuffer, uint32_t i) override;
    };

    struct Camera
    {
        glm::vec3 position;
        glm::vec3 rotation;
        float fov = glm::radians(90.f);
    };



#pragma mark -
#pragma mark Entity
    struct Entity
    {
        Context* context;
        ObjectContext vulkan;
        
        virtual void Update(const float& elapsedTime)
        {
            
        }
        
        virtual void Event(Event& event)
        {
            
        }
        
        virtual void Draw()
        {
            
        }
    };

#pragma mark -
#pragma mark Engine
    struct Engine
    {
        Window window;
        Context context;
        Event event;
        
        Camera* activeCamera = nullptr;
        
        std::vector<Drawable*> drawable;
        
        Engine()
        {
#if defined(MINEH_WINDOW_API_GLFW)
            glfwInit();
#endif
            window.create(800, 600);
            context.create(window);
            
            Sprite* sprite = new Sprite(context, "Data/Textures/79989656_p0.png");
            sprite->vertices = {
                { { -1.f,  -1.f }, { 0.0f,  0.0f } },
                { {  1.f,  -1.f }, { 1.0f,  0.0f } },
                { { -1.f,   1.f }, { 0.0f,  1.0f } },
                { {  1.f,   1.f }, { 1.0f,  1.0f } }
            };
            sprite->indices = { 0, 1, 2,   1, 3, 2 };
            sprite->init(); drawable.push_back(sprite);
            
            sprite = new Sprite(context, "Data/Textures/FUutq0IDAG8.jpg");
            sprite->vertices = {
                { { -0.5f,  -0.5f }, { 0.0f,  0.0f } },
                { {  0.5f,  -0.5f }, { 1.0f,  0.0f } },
                { { -0.5f,   0.5f }, { 0.0f,  1.0f } },
                { {  0.5f,   0.5f }, { 1.0f,  1.0f } }
            };
            sprite->indices = { 0, 1, 2,   1, 3, 2 };
            sprite->init(); drawable.push_back(sprite);
            
            context.beginRecord();
            for (auto& e : drawable)
                for (uint32_t i = 0; i < context.commandBuffers.size(); ++i)
                    e->onRecord(context.commandBuffers[i], i);
            context.endRecord();
        }
        
        ~Engine()
        {
            for (auto& e : drawable) delete e;
            context.destroy();
#if defined(MINEH_WINDOW_API_GLFW)
            glfwTerminate();
#endif
        }
        
        void Update(const float& elapsedTime)
        {
            // for (auto& e : drawable) e.Update(elapsedTime);
        }
        
        void Event(Event& event)
        {
            // for (auto& e : drawable) e.Event(event);
        }
        
        void Draw()
        {
            // for (auto& e : drawable) e.Draw();
        }
        
        void Run()
        {
            while (window.isOpen())
            {
                while (window.pollEvent(event))
                {
                    switch (event.type)
                    {
                        case Event::Type::MousePressed:
                            cout << "Event::Type::MousePressed: " << (int)event.data.mouse.button.button << " at " << event.data.mouse.button.x << " " << event.data.mouse.button.y << endl;
                            break;
                        case Event::Type::MouseReleased:
                            cout << "Event::Type::MouseReleased: " << (int)event.data.mouse.button.button << " at " << event.data.mouse.button.x << " " << event.data.mouse.button.y << endl;
                        break;
                        default: cout << "event caught with type: " << (int)event.type << endl; break;
                    }
                }
                
                Update(0);
                try { context.draw(); }
                catch (exceptions::VK_ERROR_OUT_OF_DATE_KHR& e)
                {
                    context.recreate();
                    for (auto& e : drawable) e->recreate();
                    
                    context.beginRecord();
                    for (auto& e : drawable)
                        for (uint32_t i = 0; i < context.commandBuffers.size(); ++i)
                            e->onRecord(context.commandBuffers[i], i);
                    context.endRecord();
                }
            }
            
            vkDeviceWaitIdle(context.device);
        }
    };
}

#endif /* Engine_hpp */
