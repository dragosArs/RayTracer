#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <variant>
#include <filesystem>
#include "Mesh.h"
#include "rapidobj.hpp"

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

    std::vector<Sphere> spheres;
    std::vector<Material> materials;
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
    //std::vector<std::variant<PointLight, SegmentLight, ParallelogramLight>> lightSources;
    std::vector<PointLight> lightSources;
};


struct Key {
    uint32_t posIndex;
    uint32_t normIndex;
    uint32_t texIndex;
    /*
    bool operator==(const Key& other) const {
        return (posIndex == other.posIndex && normIndex == other.normIndex && texIndex == other.texIndex);
    }
    */
};

inline bool operator==(const Key& lhs, const Key& rhs) {
    return (lhs.posIndex == rhs.posIndex) && (lhs.normIndex == rhs.normIndex) && (lhs.texIndex == rhs.texIndex);
}

template <>
struct std::hash<Key> {
    std::size_t operator()(const Key& ti) const {
        // Combine the hash of the three integers using a simple hash function
        return std::hash<int>()(ti.posIndex) ^ (std::hash<int>()(ti.normIndex) << 1) ^ (std::hash<int>()(ti.texIndex) << 2);
    }
};


// Load a prebuilt scene.
void loadScene(const std::filesystem::path& objectFilePath, const std::filesystem::path& materialFilePath, Scene& scene);
void createUniqueVertices(const rapidobj::Mesh& mesh, const rapidobj::Attributes& attributes, std::vector<Triangle>& triangles, std::vector<Vertex>& vertices);
uint32_t getIndexOfVertex(const Key& key, const rapidobj::Attributes& attributes, std::vector<Vertex>& vertices, std::unordered_map<Key, int>& uniqueIndexKeys);