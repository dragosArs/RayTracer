#pragma once
#include "Mesh.h"
#include <algorithm>

struct BVH {
	AABB boundingBox;
	int triangleIndex;
	BVH* left = nullptr;
	BVH* right = nullptr;
};

