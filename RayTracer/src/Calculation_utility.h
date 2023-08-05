#pragma once
#include "Scene.h"
#include "Ray.h"

void intersectSphere(Ray& ray, const Sphere& sphere, BasicHitInfo& hitInfo);
bool intersectSphere(const Ray& ray, const Sphere& sphere);//for shadow rays

void intersectTriangle(Ray& ray, BasicHitInfo& hitInfo, const Mesh& mesh, const Triangle& triangle);