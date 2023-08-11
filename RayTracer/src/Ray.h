#pragma once

#include <glm/glm.hpp>
#include "Scene.h"

struct Ray
{
	glm::vec3 origin = glm::vec3{ 0.0f };
	glm::vec3 direction = glm::vec3{ 0.0f, 0.0f, -1.0f };
	glm::vec3 invDirection = glm::vec3{ 0.0f, 0.0f, -1.0f };
	float t{ -1.0f };
};

struct BasicHitInfo
{
	int triangleIndex = 0;
	float barU = 0.0f;
	float barV = 0.0f;
};

struct FullHitInfo
{
	glm::vec3 position;
	glm::vec3 normal;
	//glm::vec2 texCoord;
	Material material;
};