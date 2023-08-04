#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <variant>
#include <filesystem>

struct Index {
    uint32_t positionIndex;
    uint32_t normalIndex;
    uint32_t texCoordIndex;
    uint32_t materialIndex;
};

struct Triangle {
    Index vertex0;
    Index vertex1;
    Index vertex2;
    uint32_t materialIndex{0};
};

struct Material {
    glm::vec3 kd; // Diffuse color.
    glm::vec3 ks{ 0.0f };
    float shininess{ 1.0f };
    float transparency{ 1.0f };
};

struct Plane {
    float D = 0.0f;
    glm::vec3 normal { 0.0f, 1.0f, 0.0f };
};

struct AxisAlignedBox {
    glm::vec3 lower { 0.0f };
    glm::vec3 upper { 1.0f };
};

struct Sphere {
    glm::vec3 center { 0.0f };
    float radius = 1.0f;
    Material material;
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
};

struct SegmentLight {
    glm::vec3 endpoint0, endpoint1; // Positions of endpoints
    glm::vec3 color0, color1; // Color of endpoints
};

struct ParallelogramLight {
    glm::vec3 v0; // v0
    glm::vec3 edge01, edge02; // edges from v0 to v1, and from v0 to v2
    glm::vec3 color0, color1, color2, color3;
};

//Use a vector of materials, not only to reuse materials but also pass indices insetad of full materials
struct Scene {

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<Material> materials;
    std::vector<Triangle> triangles;
    std::vector<Sphere> spheres;

    //std::vector<std::variant<PointLight, SegmentLight, ParallelogramLight>> lightSources;
    std::vector<PointLight> lightSources;
};

// Load a prebuilt scene.
void loadScene(const std::filesystem::path& objectFilePath, const std::filesystem::path& materialFilePath, Scene& scene);