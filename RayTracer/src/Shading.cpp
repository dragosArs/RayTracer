#include "Shading.h"
#include <iostream>


//std::mutex coutMutex;
glm::vec3 phongFull(const FullHitInfo& hitInfo, const Camera& camera, const PointLight& pointLight)
{

    return diffuseOnly(hitInfo, pointLight) + phongSpecularOnly(hitInfo, camera, pointLight);
}

glm::vec3 diffuseOnly(const FullHitInfo& hitInfo, const PointLight& pointLight)
{
    glm::vec3 lightDir = normalizeDirection(pointLight.position - hitInfo.position);
    float value = glm::dot(lightDir, hitInfo.normal);
    if (value < 0)
        return glm::vec3{ 0.0f };
    return hitInfo.material.kd * value * pointLight.color;
}

glm::vec3 phongSpecularOnly(const FullHitInfo& hitInfo, const Camera& camera, const PointLight& pointLight)
{
    glm::vec3 lightDir = normalizeDirection(pointLight.position - hitInfo.position);
    glm::vec3 cameraDir = normalizeDirection(camera.GetPosition() - hitInfo.position);
    glm::vec3 reflectionDir = normalizeDirection(glm::reflect(-lightDir, hitInfo.normal));

    float value = glm::dot(cameraDir, reflectionDir);
    if (value <= 0)
        return glm::vec3{ 0.0f };
    return pow(value, hitInfo.material.shininess / 4.0f) * hitInfo.material.ks *  pointLight.color;
}

glm::vec3 blinnPhongSpecularOnly(const FullHitInfo& hitInfo, const Camera& camera, const PointLight& pointLight)
{
    glm::vec3 lightDir = normalizeDirection(pointLight.position - hitInfo.position);
    glm::vec3 cameraDir = normalizeDirection(camera.GetPosition() - hitInfo.position);
    glm::vec3 halfDir = normalizeDirection(lightDir + cameraDir);

    float value = glm::dot(halfDir, hitInfo.normal);
    if (value <= 0) {
        return glm::vec3{ 0.0f };
    }
    //std::cout << "value: " << value << std::endl;
    return pow(value, hitInfo.material.shininess) * hitInfo.material.ks * pointLight.color;
}