#pragma once

#include <glm/glm.hpp>
#include "Scene.h"

struct Ray
{
	glm::vec3 origin = glm::vec3{ 0.0f };
	glm::vec3 direction = glm::vec3{ 1.0f };
	float t{ -1};
};

struct HitInfo
{
	Material material;
	glm::vec3 position = glm::vec3{ 0.0f };
	glm::vec3 normal = glm::vec3{ 0.0f };

	int object_index = 0;
};