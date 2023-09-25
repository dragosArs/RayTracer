#include "Shading.h"
#include <iostream>


std::mutex coutMutex;
glm::vec3 phongFull(const FullHitInfo& hitInfo, const Camera& camera, const PointLight& pointLight)
{

    return diffuseOnly(hitInfo, pointLight);// +blinnPhongSpecularOnly(hitInfo, camera, pointLight);
}

glm::vec3 diffuseOnly(const FullHitInfo& hitInfo, const PointLight& pointLight)
{
    std::lock_guard<std::mutex> lock(coutMutex);
    //std::cout << hitInfo.material.kd.x << " " << hitInfo.material.kd.y << " " << hitInfo.material.kd.z << std::endl;
    glm::vec3 lightDir = glm::normalize(pointLight.position - hitInfo.position);
    float value = glm::dot(lightDir, hitInfo.normal);
    if (value < 0)
        return glm::vec3{ 0.0f };
    return hitInfo.material.kd * value * pointLight.color;
}

glm::vec3 phongSpecularOnly(const FullHitInfo& hitInfo, const Camera& camera, const PointLight& pointLight)
{
    glm::vec3 lightDir = glm::normalize(pointLight.position - hitInfo.position);
    glm::vec3 cameraDir = glm::normalize(camera.GetPosition() - hitInfo.position);
    glm::vec3 reflectionDir = glm::reflect(-lightDir, hitInfo.normal);

    float value = glm::dot(cameraDir, reflectionDir);
    if (value < 0)
        return glm::vec3{ 0.0f };

    return pow(value, hitInfo.material.shininess) * hitInfo.material.ks *  pointLight.color;
}

glm::vec3 blinnPhongSpecularOnly(const FullHitInfo& hitInfo, const Camera& camera, const PointLight& pointLight)
{
    glm::vec3 lightDir = glm::normalize(pointLight.position - hitInfo.position);
    glm::vec3 cameraDir = glm::normalize(camera.GetPosition() - hitInfo.position);
    glm::vec3 reflectionDir = glm::normalize(lightDir + cameraDir);

    float value = glm::clamp(glm::dot(cameraDir, lightDir), 0.0f, 1.0f);
    if (value == 0) {
        return glm::vec3{ 0.0f };
    }

    return pow(value, hitInfo.material.shininess) * hitInfo.material.ks * pointLight.color;
}