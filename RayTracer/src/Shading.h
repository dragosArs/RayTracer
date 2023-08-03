#pragma once

#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include "Calculation_utility.h"
#include "Renderer.h"

glm::vec3 diffuseOnly(const HitInfo& hitInfo, const PointLight& pointLight);
glm::vec3 phongSpecularOnly(const HitInfo& hitInfo, const Camera& camera, const PointLight& pointLight);
glm::vec3 blinnPhongSpecularOnly(const HitInfo& hitInfo, const Camera& camera, const PointLight& pointLight);
glm::vec3 phongFull(const HitInfo& hitInfo, const Camera& camera, const PointLight& pointLight);
glm::vec3 blinnPhongFull(const HitInfo& hitInfo, const PointLight& pointLight, const glm::vec3& cameraPos);