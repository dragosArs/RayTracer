#include "Calculation_utility.h"
#include <iostream>

void intersectSphere(Ray& ray, const Sphere& sphere, HitInfo& hitInfo) {
    glm::vec3 d = ray.direction;
    glm::vec3 o = ray.origin - sphere.center;
    float P1, P2;
    float A = glm::dot(d, d);
    float B = 2.0f * glm::dot(d, o);
    float C = glm::dot(o, o) - sphere.radius * sphere.radius;
    float D = B * B - 4.0f * A * C;
    if (D < 0.0f) {
        return;
    }
    else if (D == 0) P1 = P2 = -0.5 * B / A;
    else {
        float q = (B > 0.0f) ?
            -0.5f * (B + sqrt(D)) :
            -0.5f * (B - sqrt(D));
        P1 = q / A;
        P2 = C / q;
    }
    float t = std::min(P1, P2);

    if (P1 < 0.0f) 
        t = P2;

    if (P2 < 0.0f)
        t = P1;

    if ((ray.t == - 1.0f || ray.t > t) && t > 0.0f) {
        ray.t = t;
        float a;
        hitInfo.position = ray.origin + t * ray.direction;
        if(glm::length(hitInfo.position) < 1.0f)
            a = length(hitInfo.position);
        hitInfo.material = sphere.material;
        hitInfo.normal = glm::normalize(o + t * ray.direction);
    }
}

bool intersectSphere(const Ray& ray, const Sphere& sphere) {
    glm::vec3 d = ray.direction;
    glm::vec3 o = ray.origin - sphere.center;
    float P1, P2;
    float A = glm::dot(d, d);
    float B = 2.0f * glm::dot(d, o);
    float C = glm::dot(o, o) - sphere.radius * sphere.radius;
    float D = B * B - 4.0f * A * C;
    if (D < 0.0f) {
        return false;
    }
    else if (D == 0) P1 = P2 = -0.5 * B / A;
    else {
        float q = (B > 0.0f) ?
            -0.5f * (B + sqrt(D)) :
            -0.5f * (B - sqrt(D));
        P1 = q / A;
        P2 = C / q;
    }

    return P1 >= 0.0F || P2 >= 0.0F;
}