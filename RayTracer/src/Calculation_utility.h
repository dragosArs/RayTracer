#pragma once
#include "Scene.h"
#include "Ray.h"

float intersectAABB(const Ray& ray,const AABB& aabb);
void intersectTriangle(Ray& ray, BasicHitInfo& hitInfo, const Scene& scene, const uint32_t triangleId);
bool intersectTriangle(const Ray& ray, const Scene& scene, const uint32_t triangleId, float length);
std::vector<glm::vec3> createJitter(int seed);
glm::vec3 normalizeDirection(const glm::vec3& direction);
