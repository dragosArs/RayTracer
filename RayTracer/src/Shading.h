#pragma once

#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include "Calculation_utility.h"
#include "Renderer.h"


glm::vec3 diffuseOnly(const FullHitInfo& hitInfo, const PointLight& pointLight);
glm::vec3 phongSpecularOnly(const FullHitInfo& hitInfo, const Camera& camera, const PointLight& pointLight);
glm::vec3 blinnPhongSpecularOnly(const FullHitInfo& hitInfo, const Camera& camera, const PointLight& pointLight);
glm::vec3 phongFull(const FullHitInfo& hitInfo, const Camera& camera, const PointLight& pointLight);
glm::vec3 blinnPhongFull(const FullHitInfo& hitInfo, const PointLight& pointLight, const glm::vec3& cameraPos);