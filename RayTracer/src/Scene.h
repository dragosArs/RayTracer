#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <filesystem>
#include "Bvh.h"
#include "rapidobj.hpp"

const float EPSILON = 0.00001f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
};

struct SegmentLight {
    glm::vec3 endpoint0, endpoint1; // Positions of endpoints
    glm::vec3 color0, color1; // Color of endpoints
};

struct ParallelogramLight {
    glm::vec3 v0;
    glm::vec3 edge1, edge2;
    glm::vec3 color0, color1, color2, color3;
    std::vector<PointLight> samples;
};
 
//struct ParallelogramLight {
//    glm::vec3 v0; // v0
//    glm::vec3 edge01, edge02; // edges from v0 to v1, and from v0 to v2
//    glm::vec3 color0, color1, color2, color3;
//};

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

//Use a vector of materials, not only to reuse materials but also pass indices insetad of full materials
struct Scene {
    std::vector<Material> materials;
    std::vector<Vertex> vertices;
    std::vector<Texture> diffuseMaps;
    std::vector<Texture> normalMaps;
    std::vector<Triangle> triangles;
    std::unique_ptr<BVH> bvhTree;
    std::vector<bvhNode> flatBvh;
    std::vector<PointLight> pointLightSources;
    std::vector<ParallelogramLight> parallelogramLightSources;

    // Load a prebuilt scene.
    void load(const std::filesystem::path& objectFilePath, const std::filesystem::path& materialFilePath);
    void loadMaterials(const std::vector<rapidobj::Material>& materials);
    void updateDiffuseMap(Material& material, const std::string& diffuse_texname, std::unordered_map<std::string, int>& texMap);
    void updateNormalMap(Material& material, const std::string& normal_texname, std::unordered_map<std::string, int>& texMap);
    void createUniqueVertices(const rapidobj::Mesh& mesh, const rapidobj::Attributes& attributes);
    uint32_t getIndexOfVertex(const Key& key, const rapidobj::Attributes& attributes, std::unordered_map<Key, int>& uniqueIndexKeys);
    std::unique_ptr<BVH> prepBvh(int left, int right, const AABB& box);
    void  Scene::flattenBvh(const std::unique_ptr<BVH>& bvhTree, std::vector<bvhNode>& flatBvh);
    int splitWithSAH(const AABB& box, const glm::vec3& boxDimensions, glm::vec3& splitPointLeft, glm::vec3& splitPointRight, int left, int right, int axis);
    void sampleAreaLights(int detail);
    //std::unique_ptr<BVH> Scene::prepBvh(int left, int right, int level);
};


AABB createAABBForTriangle(const Triangle& triangle, std::vector<Vertex>& vertices);
AABB combineAABBs(const AABB& box1, const AABB& box2);
std::vector<glm::vec3> loadTexture(std::shared_ptr<unsigned char> imageData, int width, int height, int numChannels);





