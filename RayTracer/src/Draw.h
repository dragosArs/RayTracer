#pragma once
#include "Ray.h"
#include "Mesh.h"
#include "Calculation_utility.h"


std::vector<std::pair<glm::vec3, glm::vec3>> DrawBvh(const BVH* bvh);
std::vector<std::pair<glm::vec3, glm::vec3>> DrawMeshes(const Scene& scene);
