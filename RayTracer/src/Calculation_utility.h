#pragma once
#include "Scene.h"
#include "Ray.h"

float intersectAABB(const Ray& ray,const AABB& aabb);
void intersectTriangle(Ray& ray, BasicHitInfo& hitInfo, const Scene& scene, const uint32_t triangleId);
bool intersectTriangle(const Ray& ray, const Scene& scene, const uint32_t triangleId, float length);
glm::vec3 normalizeDirection(const glm::vec3& direction);
