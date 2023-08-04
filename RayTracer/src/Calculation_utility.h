#pragma once
#include "Scene.h"
#include "Ray.h"

void intersectSphere(Ray& ray, const Sphere& sphere, HitInfo& hitInfo);
bool intersectSphere(const Ray& ray, const Sphere& sphere);//for shadow rays

void intersectTriangle(Ray& ray, const Vertex& v0, const Vertex& v1, const Vertex& v2, HitInfo& hitInfo);