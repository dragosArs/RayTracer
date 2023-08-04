#include "Shading.h"
#include <iostream>

glm::vec3 phongFull(const HitInfo& hitInfo, const Camera& camera, const PointLight& pointLight)
{
    return diffuseOnly(hitInfo, pointLight) + blinnPhongSpecularOnly(hitInfo, camera, pointLight);
}

glm::vec3 diffuseOnly(const HitInfo& hitInfo, const PointLight& pointLight)
{
    glm::vec3 lightDir = glm::normalize(pointLight.position - hitInfo.position);
    float value = glm::dot(lightDir, hitInfo.normal);
    if (value < 0)
        return glm::vec3(0.0f, 0.0f, 0.0f);
    return hitInfo.material.kd * value * pointLight.color;
}

glm::vec3 phongSpecularOnly(const HitInfo& hitInfo, const Camera& camera, const PointLight& pointLight)
{
    glm::vec3 lightDir = glm::normalize(pointLight.position - hitInfo.position);
    glm::vec3 cameraDir = glm::normalize(camera.GetPosition() - hitInfo.position);
    glm::vec3 reflectionDir = glm::reflect(-lightDir, hitInfo.normal);

    float value = glm::dot(cameraDir, reflectionDir);
    if (value < 0)
        return glm::vec3(0.0f, 0.0f, 0.0f);
    
    float shininess = hitInfo.material.shininess;
    //float shininess = 1.0f;
    //because value is between 0 and 1, bigger shininess means smaller specular
    //TODO find a way to make it more realistic, tweak variables 
    return pow(value, 50.0f) * hitInfo.material.ks *  pointLight.color;
}

glm::vec3 blinnPhongSpecularOnly(const HitInfo& hitInfo, const Camera& camera, const PointLight& pointLight)
{
    glm::vec3 lightDir = glm::normalize(pointLight.position - hitInfo.position);
    glm::vec3 cameraDir = glm::normalize(camera.GetPosition() - hitInfo.position);
    glm::vec3 reflectionDir = glm::normalize(lightDir + cameraDir);

    float value = glm::dot(cameraDir, lightDir);
    if (value < 0) {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
    float shininess = hitInfo.material.shininess;
    //TODO find a way to make it more realistic, tweak variables 
    return pow(value, 100.0f) * hitInfo.material.ks * pointLight.color;
}