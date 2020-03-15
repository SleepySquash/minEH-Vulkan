//
//  Vertex.hpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 11.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#ifndef Vertex_hpp
#define Vertex_hpp

#include <vector>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace mh
{
    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };


    struct Vertex2D
    {
        glm::vec2 pos;
        glm::vec2 uv;
        
        bool operator==(const Vertex2D& other) const;
    };


    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 col;
        glm::vec2 texCoords;
        
        bool operator==(const Vertex& other) const;
    };
}

namespace std
{
    template<> struct hash<mh::Vertex>
    {
        size_t operator()(mh::Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.col) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoords) << 1);
        }
    };

    template<> struct hash<mh::Vertex2D>
    {
        size_t operator()(mh::Vertex2D const& vertex) const
        {
            return ((hash<glm::vec2>()(vertex.pos) ^
                   (hash<glm::vec2>()(vertex.uv) << 1)) >> 1);
        }
    };
}

#endif /* Vertex_hpp */
