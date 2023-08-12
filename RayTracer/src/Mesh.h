#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

struct AABB {
    glm::vec3 lower;
    glm::vec3 upper;
};

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
    glm::vec3 centroid;
};

struct Material {
    glm::vec3 kd; // Diffuse color.
    glm::vec3 ks{ 0.0f };
    float shininess{ 20.0f };
    float transparency{ 1.0f };

    friend std::ostream& operator<<(std::ostream& os, const Material& v);
};

glm::vec3 getTriangleCentroid(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3);