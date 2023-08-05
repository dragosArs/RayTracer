#pragma once

#include <glm/glm.hpp>
#include "Scene.h"

struct Ray
{
	glm::vec3 origin = glm::vec3{ 0.0f };
	glm::vec3 direction = glm::vec3{ 1.0f };
	float t{ -1};
};

struct BasicHitInfo
{
	int triangleIndex = 0;
	int meshIndex = 0;
	int barU = 0;
	int barV = 0;
};

struct FullHitInfo
{
	glm::vec3 position;
	glm::vec3 normal;
	//glm::vec2 texCoord;
	Material material;
};