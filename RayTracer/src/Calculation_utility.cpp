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
        hitInfo.position = ray.origin + t * ray.direction;
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
    else if (D == 0.0f) P1 = P2 = -0.5f * B / A;
    else {
        float q = (B > 0.0f) ?
            -0.5f * (B + sqrt(D)) :
            -0.5f * (B - sqrt(D));
        P1 = q / A;
        P2 = C / q;
    }

    return P1 >= 0.0F || P2 >= 0.0F;
}

void triangleNormal(const glm::vec3& posV0, const glm::vec3& posV1, const glm::vec3& posV2, glm::vec3& normal) {
	glm::vec3 e1 = posV1 - posV0;
	glm::vec3 e2 = posV2 - posV0;
	normal = glm::normalize(glm::cross(e1, e2));
}

void intersectsTriangle(Ray& ray, const Vertex& v0, const Vertex& v1, const Vertex& v2, HitInfo& hitInfo) {
	glm::vec3 posV0 = v0.position;
	glm::vec3 posV1 = v1.position;
	glm::vec3 posV2 = v2.position;
	glm::vec3 normal;
	triangleNormal(posV0, posV1, posV2, normal);
	float t = glm::dot(posV0 - ray.origin, normal) / glm::dot(ray.direction, normal);
	if (t < 0.0f) return;
	glm::vec3 p = ray.origin + t * ray.direction;
	glm::vec3 c;
	glm::vec3 edge0 = posV1 - posV0;
	glm::vec3 vp0 = p - posV0;
	c = glm::cross(edge0, vp0);
	if (glm::dot(normal, c) < 0.0f) return;
	glm::vec3 edge1 = posV2 - posV1;
	glm::vec3 vp1 = p - posV1;
	c = glm::cross(edge1, vp1);
	if (glm::dot(normal, c) < 0.0f) return;
	glm::vec3 edge2 = posV0 - posV2;
	glm::vec3 vp2 = p - posV2;
	c = glm::cross(edge2, vp2);
	if (glm::dot(normal, c) < 0.0f) return;
    if (ray.t == -1.0f || ray.t > t) {
		ray.t = t;
		hitInfo.position = p;
		hitInfo.normal = normal;
		//hitInfo.material = triangle.material;
	}
}