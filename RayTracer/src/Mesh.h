#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;

    friend std::ostream& operator<<(std::ostream& os, const Vertex& v);
};





struct Triangle {
    uint32_t vertexIndex0;
    uint32_t vertexIndex1;
    uint32_t vertexIndex2;
    uint32_t materialIndex;
};

struct Material {
    glm::vec3 kd; // Diffuse color.
    glm::vec3 ks{ 0.0f };
    float shininess{ 20.0f };
    float transparency{ 1.0f };

    friend std::ostream& operator<<(std::ostream& os, const Material& v);
};