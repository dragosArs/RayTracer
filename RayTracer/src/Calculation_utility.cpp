#include "Calculation_utility.h"
#include <iostream>
#include <immintrin.h>                                                                          

//std::mutex coutMutex;
//float intersectAABB(const Ray& ray, const AABB& box, bool debug)
//{
//    glm::vec3 tMin = (box.lower - ray.origin) * ray.invDirection;
//    glm::vec3 tMax = (box.upper - ray.origin) * ray.invDirection;
//    glm::vec3 t1 = glm::min(tMin, tMax);
//    glm::vec3 t2 = glm::max(tMin, tMax);
//
//    float tNear = std::max(std::max(t1.x, t1.y), t1.z);
//    float tFar = std::min(std::min(t2.x, t2.y), t2.z);
//
//
//    //std::lock_guard<std::mutex> lock(coutMutex);
//    if (tNear < tFar)
//        std::cout << tNear << "\n";
//    /*std::cout << "lower: " << box.lower.x << " " << box.lower.y << " " << box.lower.z << "\n";
//    std::cout << "upper: " << box.upper.x << " " << box.upper.y << " " << box.upper.z << "\n";
//    std::cout << tNear << tFar << "\n";*/
//    if (tNear > EPSILON && tNear < tFar) {
//        return tNear;
//    }
//    else if (tNear < EPSILON && tFar > EPSILON) {
//        return tFar;
//    }
//    else
//        return -1.0f;
//
//}


float intersectAABB(const Ray& ray, const AABB& box, bool debug)
{
    glm::vec3 tMin = (box.lower - ray.origin) * ray.invDirection;
    glm::vec3 tMax = (box.upper - ray.origin) * ray.invDirection;
    glm::vec3 t1 = glm::min(tMin, tMax);
    glm::vec3 t2 = glm::max(tMin, tMax);

    float tNear = std::max(std::max(t1.x, t1.y), t1.z);
    float tFar = std::min(std::min(t2.x, t2.y), t2.z);

    if (tNear > tFar)
        return -1.0f;
    else if (tNear > EPSILON)
        return tNear;
    else if (tFar > EPSILON)
        return tFar;
    else
        return -1.0f;

}

void intersectTriangle(Ray& ray, BasicHitInfo& hitInfo, const Scene& scene, const uint32_t triangleId, bool debug)
{
    Triangle triangle = scene.triangles[triangleId];
    glm::vec3 posVertex0 = scene.vertices[triangle.vertexIndex0].position;
    glm::vec3 posVertex1 = scene.vertices[triangle.vertexIndex1].position;
    glm::vec3 posVertex2 = scene.vertices[triangle.vertexIndex2].position;
    glm::vec3 edge1, edge2, h, s, q;
    float a, f, u, v;
    edge1 = posVertex1 - posVertex0;
    edge2 = posVertex2 - posVertex0;
    h = glm::cross(ray.direction, edge2);
    a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        return;    // This ray is parallel to this triangle.

    f = 1.0f / a;
    s = ray.origin - posVertex0;
    u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f)
       return;

    q = glm::cross(s, edge1);
    v = f * glm::dot(ray.direction, q);

    if (v < 0.0f || u + v > 1.0f)
        return;

    float t = f * glm::dot(edge2, q);

    if(t > EPSILON && t < ray.t) {
        ray.t = t;
        hitInfo.triangleIndex = triangleId;
        hitInfo.barU = u;
        hitInfo.barV = v;
    }
}

//This is specifically used for shadows, where we only need to know if there is an intersection, and not the exact point of intersection.
bool intersectTriangle(const Ray& ray, const Scene& scene, const uint32_t triangleId, float length, bool debug)
{
    Triangle triangle = scene.triangles[triangleId];
    glm::vec3 posVertex0 = scene.vertices[triangle.vertexIndex0].position;
    glm::vec3 posVertex1 = scene.vertices[triangle.vertexIndex1].position;
    glm::vec3 posVertex2 = scene.vertices[triangle.vertexIndex2].position;
    glm::vec3 edge1, edge2, h, s, q;
    float a, f, u, v;
    edge1 = posVertex1 - posVertex0;
    edge2 = posVertex2 - posVertex0;
    h = glm::cross(ray.direction, edge2);
    a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        false;    // This ray is parallel to this triangle.

    f = 1.0f / a;
    s = ray.origin - posVertex0;
    u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    q = glm::cross(s, edge1);
    v = f * glm::dot(ray.direction, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    float t = f * glm::dot(edge2, q); 
    return t > EPSILON && t + EPSILON < length;
}


    


