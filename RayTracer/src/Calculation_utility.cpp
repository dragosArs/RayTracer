#include "Calculation_utility.h"
#include <iostream>

void intersectSphere(Ray& ray, const Sphere& sphere, BasicHitInfo& hitInfo) {
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
    else if (D == 0.0f) P1 = P2 = -0.5f * B / A;
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
        /*
        hitInfo.position = ray.origin + t * ray.direction;
        hitInfo.material = sphere.material;
        hitInfo.normal = glm::normalize(o + t * ray.direction);
        */
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
    else if (D == 0.0f) P1 = P2 = -0.5f * B / A;
    else {
        float q = (B > 0.0f) ?
            -0.5f * (B + sqrt(D)) :
            -0.5f * (B - sqrt(D));
        P1 = q / A;
        P2 = C / q;
    }

    return P1 >= 0.0f || P2 >= 0.0f;
}


void intersectTriangle(Ray& ray, BasicHitInfo& hitInfo, const Mesh& mesh, const uint32_t triangleId) {
    const float EPSILON = 0.0000001f;
    Triangle triangle = mesh.triangles[triangleId];
    glm::vec3 posVertex0 = mesh.vertices[triangle.vertexIndex0].position;
    glm::vec3 posVertex1 = mesh.vertices[triangle.vertexIndex1].position;
    glm::vec3 posVertex2 = mesh.vertices[triangle.vertexIndex2].position;
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

    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * glm::dot(edge2, q);

    // ray intersection
    //TODO: it is better to just keep track of the intersected triangle index(and u and v for barycentric interpolation), and let the shader access the relevant primtiives,
    //That's because the shader is called at most &bounces times per pixel, while the update conditon below is called every time we intersect a triangle that is closer than the current one.
    //What if there are 10 triangles that intersect the ray, but we only want to keep the closest one? We would have to update the ray.t 10 times.
    if(t > EPSILON && (ray.t == -1.0f || ray.t > t)) {
        //spent too much debugging this... I wasn't updating ray.t:(((((((
        ray.t = t;
        hitInfo.triangleIndex = triangleId;
    }
}
