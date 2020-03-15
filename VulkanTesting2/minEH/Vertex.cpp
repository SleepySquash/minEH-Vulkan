//
//  Vertex.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 11.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "Vertex.hpp"

namespace mh
{
    bool Vertex2D::operator==(const Vertex2D& other) const { return pos == other.pos && uv == other.uv; }
    bool Vertex::operator==(const Vertex& other) const { return pos == other.pos && col == other.col && texCoords == other.texCoords; }
}
