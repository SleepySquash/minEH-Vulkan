//
//  CameraController.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 23.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "CameraController.hpp"

namespace mh
{
    void FreeCameraController::init(Window& window, Camera& camera) { this->window = &window; this->camera = &camera; }
    void FreeCameraController::Update(const float& elapsedTime)
    {
        if (moveUp || moveDown || moveS || moveD || moveW || moveA || rotA || rotD || rotW || rotS)
        {
            if (moveUp)        camera->position.y += cameraSpeed * elapsedTime;
            else if (moveDown) camera->position.y -= cameraSpeed * elapsedTime;
            
            /// glm::normalize(glm::vec3( camera->direction.x, 0.f, camera->direction.z ))     <- on xz
            if (fps)
            {
                if (moveW)      camera->position -= glm::normalize(glm::vec3( camera->direction.x, 0.f, camera->direction.z )) * cameraSpeed * elapsedTime;
                else if (moveS) camera->position += glm::normalize(glm::vec3( camera->direction.x, 0.f, camera->direction.z )) * cameraSpeed * elapsedTime;
                if (moveA)      camera->position -= glm::normalize(glm::cross(camera->up, glm::normalize(glm::vec3( camera->direction.x, 0.f, camera->direction.z )))) * cameraSpeed * elapsedTime;
                else if (moveD) camera->position += glm::normalize(glm::cross(camera->up, glm::normalize(glm::vec3( camera->direction.x, 0.f, camera->direction.z )))) * cameraSpeed * elapsedTime;
            }
            else
            {
                if (moveW)      camera->position -= camera->direction * cameraSpeed * elapsedTime;
                else if (moveS) camera->position += camera->direction * cameraSpeed * elapsedTime;
                if (moveA)      camera->position -= glm::normalize(glm::cross(camera->up, camera->direction)) * cameraSpeed * elapsedTime;
                else if (moveD) camera->position += glm::normalize(glm::cross(camera->up, camera->direction)) * cameraSpeed * elapsedTime;
            }
            
            if (rotA || rotD || rotW || rotS)
            {
                if (rotA)      yaw -= cameraSensitivity * 500 * elapsedTime;
                else if (rotD) yaw += cameraSensitivity * 500 * elapsedTime;
                if (rotW)      pitch -= cameraSensitivity * 500 * elapsedTime;
                else if (rotS) pitch += cameraSensitivity * 500 * elapsedTime;
                
                camera->direction.x = cosf(yaw) * cosf(pitch);
                camera->direction.y = sinf(pitch);
                camera->direction.z = sinf(yaw) * cosf(pitch);
                camera->direction = glm::normalize(camera->direction);
            }
            
            camera->update();
        }
    }
    void FreeCameraController::OnEvent(Event& event)
    {
        switch (event.type)
        {
            case Event::Type::KeyPressed:
            case Event::Type::keyReleased:
                switch (event.data.key)
                {
                    case Keyboard::Key::A: moveA = (event.type == Event::Type::KeyPressed); break;
                    case Keyboard::Key::D: moveD = (event.type == Event::Type::KeyPressed); break;
                    case Keyboard::Key::W: moveW = (event.type == Event::Type::KeyPressed); break;
                    case Keyboard::Key::S: moveS = (event.type == Event::Type::KeyPressed); break;
                    case Keyboard::Key::Q: moveDown = (event.type == Event::Type::KeyPressed); break;
                    case Keyboard::Key::E: moveUp = (event.type == Event::Type::KeyPressed); break;
                    case Keyboard::Key::Up: rotW = (event.type == Event::Type::KeyPressed); break;
                    case Keyboard::Key::Down: rotS = (event.type == Event::Type::KeyPressed); break;
                    case Keyboard::Key::Right: rotD = (event.type == Event::Type::KeyPressed); break;
                    case Keyboard::Key::Left: rotA = (event.type == Event::Type::KeyPressed); break;
                    case Keyboard::Key::LShift: cameraSpeed = (event.type == Event::Type::KeyPressed) ? cameraSpeedFast : cameraSpeedSlow; break;
                    default: break;
                }
                break;
            case Event::Type::MousePressed: if (!mouseLockEnabled) SwitchInputMode(); break;
            case Event::Type::MouseMove:
                if (!mouseLockEnabled) return;
                if (firstMouse)
                {
                    lastxpos = event.data.mouse.move.x;
                    lastypos = event.data.mouse.move.y;
                    firstMouse = false;
                }
                mouseMoveOffsetx = event.data.mouse.move.x - lastxpos;
                mouseMoveOffsety = event.data.mouse.move.y - lastypos;
                lastxpos = event.data.mouse.move.x;
                lastypos = event.data.mouse.move.y;

                mouseMoveOffsetx *= cameraSensitivity;
                mouseMoveOffsety *= cameraSensitivity;

                yaw   += mouseMoveOffsetx;
                pitch += mouseMoveOffsety;

                if (pitch > 1.57f) pitch = 1.57f;
                else if (pitch < -1.57f) pitch = -1.57f;

                camera->direction.x = cosf(yaw) * cosf(pitch);
                camera->direction.y = sinf(pitch);
                camera->direction.z = sinf(yaw) * cosf(pitch);
                camera->direction = glm::normalize(camera->direction);
                
                camera->update();
                break;
            default: break;
        }
        switch (event.type)
        {
            case Event::Type::KeyPressed:
                switch (event.data.key)
                {
                    case Keyboard::Key::Escape: SwitchInputMode(); break;
                    default: break;
                }
                break;
            default: break;
        }
    }
    void FreeCameraController::SwitchInputMode()
    {
        if (mouseLockEnabled)
        {
            mouseLockEnabled = false; firstMouse = true;
            window->setCursorMode(Mouse::CursorMode::Normal);
        }
        else
        {
            mouseLockEnabled = firstMouse = true;
            window->setCursorMode(Mouse::CursorMode::Disabled);
        }
    }
}
