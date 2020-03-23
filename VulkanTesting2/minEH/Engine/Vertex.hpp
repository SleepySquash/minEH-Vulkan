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

#include <glm/fwd.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace mh
{
    template<typename T> struct Vertex
    {
        T pos;
        glm::vec2 uv;
        
        bool operator==(const Vertex<T>& other) const { return pos == other.pos && uv == other.uv; }
    };
}

namespace std
{
    template<typename T> struct hash<mh::Vertex<T>>
    {
        size_t operator()(mh::Vertex<T> const& vertex) const
        {
            return ((hash<T>()(vertex.pos) ^
                   (hash<glm::vec2>()(vertex.uv) << 1)) >> 1);
        }
    };
}

#endif /* Vertex_hpp */
