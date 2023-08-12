#pragma once
#include "Mesh.h"
#include <algorithm>

struct BVH {
	AABB boundingBox;
	int triangleIndex;
	std::unique_ptr<BVH> left = nullptr;
	std::unique_ptr<BVH> right = nullptr;
};

