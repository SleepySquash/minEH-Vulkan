//
//  ObjectContext.cpp
//  VulkanTesting2
//
//  Created by Никита Исаенко on 14.03.2020.
//  Copyright © 2020 Melancholy Hill. All rights reserved.
//

#include "ObjectContext.hpp"

#include <tiny_obj_loader.h>

namespace mh
{

#pragma mark -
#pragma mark PerObject

    void ObjectContext::loadModel(const std::string& path)
    {
        if (path.length() == 0) return;
        
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) throw std::runtime_error("loadModel() failed: " + warn + err);
        
        std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
        
        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex;
                
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoords = {
                          attrib.texcoords[2 * index.texcoord_index + 0],
                    1.f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.col = { 1.0f, 1.0f, 1.0f };
                
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex); }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

}
