#pragma once
#include "Scene.h"
#include "Ray.h"

//void intersectSphere(Ray& ray, const Sphere& sphere, BasicHitInfo& hitInfo);
//bool intersectSphere(const Ray& ray, const Sphere& sphere);//for shadow rays

bool intersectAABB(const Ray& ray,const AABB& aabb);

void intersectTriangle(Ray& ray, BasicHitInfo& hitInfo, const Scene& scene, const uint32_t triangle);
bool intersectTriangle(const Ray& ray, const Scene& scene, const uint32_t triangleId);
