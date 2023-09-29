#include "Calculation_utility.h"
#include <iostream>                                                                        
#include <random>
#include <glm/gtx/quaternion.hpp>

std::mutex coutMutex;
float intersectAABB(const Ray& ray, const AABB& box)
{
    const glm::vec3 tMin = (box.lower - ray.origin) * ray.invDirection;
    const glm::vec3 tMax = (box.upper - ray.origin) * ray.invDirection;
    const glm::vec3 t1 = glm::min(tMin, tMax);
    const glm::vec3 t2 = glm::max(tMin, tMax);

    const float tNear = std::max(std::max(t1.x, t1.y), t1.z);
    const float tFar = std::min(std::min(t2.x, t2.y), t2.z);



    /*if (tNear > tFar)
        return -1.0f;
    else if (tNear > EPSILON)
        return tNear;
    else if (tFar > EPSILON)
        return tFar;
    else
        return -1.0f;*/

    if (tNear > tFar)
        return -1.0f;
    else if (tNear > EPSILON)
        return tNear;
    else
        return tFar;

}

void intersectTriangle(Ray& ray, BasicHitInfo& hitInfo, const Scene& scene, const uint32_t triangleId)
{
    const Triangle& triangle = scene.triangles[triangleId];
    const glm::vec3& posVertex0 = scene.vertices[triangle.vertexIndex0].position;
    const glm::vec3& posVertex1 = scene.vertices[triangle.vertexIndex1].position;
    const glm::vec3& posVertex2 = scene.vertices[triangle.vertexIndex2].position;
    //glm::vec3 edge1, edge2, h, s, q;
    //float a, f, u, v;
    const glm::vec3 edge1 = posVertex1 - posVertex0;
    const glm::vec3 edge2 = posVertex2 - posVertex0;
    const glm::vec3 h = glm::cross(ray.direction, edge2);
    const float a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        return;    // This ray is parallel to this triangle.

    const float f = 1.0f / a;
    const glm::vec3 s = ray.origin - posVertex0;
    const float u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f)
       return;

    const glm::vec3 q = glm::cross(s, edge1);
    const float v = f * glm::dot(ray.direction, q);

    if (v < 0.0f || u + v > 1.0f)
        return;

    const float t = f * glm::dot(edge2, q);

    if(t > EPSILON && t < ray.t) {
        ray.t = t;
        hitInfo.triangleIndex = triangleId;
        hitInfo.barU = u;
        hitInfo.barV = v;
    }
}


//returns coplanar vectors that represent random points on a unit circle(when applied magnitude can be scaled)
//they can be used to create different effects: glossy reflections, depth of field, etc.
//std::vector<glm::vec3> createJitter(const int seed)
//{
//    std::vector<glm::vec3> jitterSamples;
//    //TODO CHANGE THIS
//    constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
//    glm::vec3 rt{ 0.0f, 0.0f, -1.0f };
//    std::mt19937 rng(12345);
//
//    // Create a distribution for random values between 0 and 1
//    std::uniform_real_distribution<double> distribution(0.0, 1.0);
//    glm::quat rotation = glm::angleAxis(glm::radians(360.0f / 52.5f), upDirection);
//
//    for (int i = 0; i < 100; i++)
//    {
//        float x = distribution(rng);
//        
//        rt = rotation * rt;
//        jitterSamples.push_back(x * rt);
//    } 
//
//    return jitterSamples;
//
//}

std::vector<glm::vec3> createJitter(const int seed)
{
    std::vector<glm::vec3> jitterSamples;
    //TODO CHANGE THIS
    // Seed for the random number generator
    //unsigned int seed = 42; // Change this to your desired seed value

    // Create random number generators with the given seed
    std::default_random_engine generator(seed);

    // Define the distributions for magnitude (radius) and angle
    std::normal_distribution<double> magnitude_dist(1.0, 0.2); // Mean: 1.0, Standard Deviation: 0.2
    std::uniform_real_distribution<double> angle_dist(0.0, 2.0 * glm::pi<float>()); // Uniform angle distribution (0 to 2 * pi)
    

    // Number of points to generate
    int num_points = 100; // Adjust as needed

    // Generate and print random points on the unit circle
    for (int i = 0; i < num_points; ++i) {
        // Generate random magnitude (radius) and angle
        float magnitude = 1.0 / (1.0 + glm::exp(magnitude_dist(generator)));
        float angle = angle_dist(generator);

        // Calculate the Cartesian coordinates (x, y) from polar coordinates
        float x = magnitude * cos(angle);
        double y = magnitude * sin(angle);

        jitterSamples.push_back(glm::vec3(x, 0.0f, y));
    }

    return jitterSamples;

}

//This is specifically used for shadows, where we only need to know if there is an intersection, and not the exact point of intersection.
bool intersectTriangle(const Ray& ray, const Scene& scene, const uint32_t triangleId, float length)
{
    Triangle triangle = scene.triangles[triangleId];
    const glm::vec3& posVertex0 = scene.vertices[triangle.vertexIndex0].position;
    const glm::vec3& posVertex1 = scene.vertices[triangle.vertexIndex1].position;
    const glm::vec3& posVertex2 = scene.vertices[triangle.vertexIndex2].position;
    //const glm::vec3 edge1, edge2, h, s, q;
    //float a, f, u, v;
    const glm::vec3 edge1 = posVertex1 - posVertex0;
    const glm::vec3 edge2 = posVertex2 - posVertex0;
    const glm::vec3 h = glm::cross(ray.direction, edge2);
    const float a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        false;    // This ray is parallel to this triangle.

    const float f = 1.0f / a;
    const glm::vec3  s = ray.origin - posVertex0;
    const float u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    const glm::vec3 q = glm::cross(s, edge1);
    const float v = f * glm::dot(ray.direction, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    float t = f * glm::dot(edge2, q); 
    return t > EPSILON && t + EPSILON < length;
}

glm::vec3 normalizeDirection(const glm::vec3& direction)
{
    const float& length = glm::length(direction);
    return length > EPSILON ? direction / length : direction;
}


    


