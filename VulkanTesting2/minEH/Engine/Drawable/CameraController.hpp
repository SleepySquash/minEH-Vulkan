//
//  CameraController.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 23.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef CameraController_hpp
#define CameraController_hpp

#include <iostream>
#include <vector>

#include "Camera.hpp"
#include "../Window.hpp"

namespace mh
{
    /// Camera thanks to: https://learnopengl.com/Getting-started/Camera
    struct FreeCameraController
    {
        Camera* camera;
        Window* window;

        bool fps = false;
        bool moveA{ false }, moveD{ false }, moveW{ false }, moveS{ false }, moveUp{ false }, moveDown{ false };
        bool rotA{ false }, rotD{ false }, rotW{ false }, rotS{ false };

        bool mouseLockEnabled{ false }, firstMouse{ false };
        float lastxpos, lastypos, mouseMoveOffsetx, mouseMoveOffsety;

        float cameraSpeedSlow{ 0.8f }, cameraSpeedFast{ 4.f }, cameraSensitivity{ 0.002f };
        float cameraSpeed{ cameraSpeedSlow }, pitch{ 0 }, yaw{ glm::radians(90.f) };

        void init(Window& window, Camera& camera);
        void Update(const float& elapsedTime);
        void OnEvent(Event& event);
        void SwitchInputMode();
    };
}

#endif /* CameraController_hpp */
