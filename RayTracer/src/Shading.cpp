#include "Shading.h"

glm::vec3 phongFull(const HitInfo& hitInfo, const glm::vec3& cameraPos, const PointLight& pointLight)
{
    return diffuseOnly(hitInfo, pointLight);// +phongSpecularOnly(hitInfo, cameraPos, pointLight);
}

glm::vec3 diffuseOnly(const HitInfo& hitInfo, const PointLight& pointLight)
{
    glm::vec3 l = pointLight.position - hitInfo.position;
    float value = glm::dot(glm::normalize(l), hitInfo.normal);
    if (value < 0)
        return glm::vec3(0.0f, 0.0f, 0.0f);
    return hitInfo.material.GetDiffuse() * value * pointLight.color;
}

glm::vec3 phongSpecularOnly(const HitInfo& hitInfo, const glm::vec3& cameraPos, const PointLight& pointLight)
{
    glm::vec3 l = glm::normalize(pointLight.position - hitInfo.position);
    glm::vec3 c = glm::normalize(cameraPos - hitInfo.position);
    glm::vec3 r = glm::normalize(-l + (2 * (glm::dot(l, hitInfo.normal)) * hitInfo.normal));

    float value = glm::dot(c, r);
    if (value < 0) {
        value = 0;
    }
    float shininess = (1 - hitInfo.material.Roughness) * (1 - hitInfo.material.Roughness);
    return hitInfo.material.GetSpecular() * pow(value, shininess) * pointLight.color;
    //return glm::vec3(0, 1, 0);
}