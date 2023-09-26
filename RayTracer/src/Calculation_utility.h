#pragma once
#include "Scene.h"
#include "Ray.h"

float intersectAABB(const Ray& ray,const AABB& aabb, bool debug);
void intersectTriangle(Ray& ray, BasicHitInfo& hitInfo, const Scene& scene, const uint32_t triangleId, bool debug);
bool intersectTriangle(const Ray& ray, const Scene& scene, const uint32_t triangleId, float length, bool debug);
