#pragma once
#include "Scene.h"
#include "Ray.h"

void intersectSphere(Ray& ray, const Sphere& sphere, HitInfo& hitInfo);
bool intersectSphere(const Ray& ray, const Sphere& sphere);//for shadow rays

void intersectTriangle(Ray& ray, HitInfo& hitInfo, const Scene& scene, const Triangle& triangle);