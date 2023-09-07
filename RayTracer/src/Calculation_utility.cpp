#include "Calculation_utility.h"
#include <iostream>
#include <immintrin.h>


bool intersectAABB(const Ray& ray, const AABB& box, bool debug)
{
    glm::vec3 tMin = (box.lower - ray.origin) * ray.invDirection;
    glm::vec3 tMax = (box.upper - ray.origin) * ray.invDirection;
    glm::vec3 t1 = glm::min(tMin, tMax);
    glm::vec3 t2 = glm::max(tMin, tMax);

    float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
    float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

    return tFar >= tNear && tFar >= EPSILON;
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
    if(debug)
        std::cout << "t: " << t << std::endl;
    //The shader is called once per ray that successfuly hits an object, while the update conditon below is called every time we intersect a triangle that is closer than the current one.
    //What if there are 10 triangles that intersect the ray, while we are only interested in the closest one? We would have to update the full hit info 10 times. Better to perform the update
    //on a smaller data structure, and only update the full hit info once we know we have the closest triangle.
    if(t > EPSILON && (ray.t == -1.0f || ray.t > t)) {
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
    //return t > EPSILON && t + EPSILON < length;
    
    if (t > EPSILON && t + EPSILON < length) {
        glm::vec3 normal = glm::normalize((1 - u - v) * scene.vertices[triangle.vertexIndex0].normal +
            u * scene.vertices[triangle.vertexIndex1].normal + v * scene.vertices[triangle.vertexIndex2].normal);
        return glm::dot(ray.direction, normal) > 0;
    }
    
    return false;
    
}
    


