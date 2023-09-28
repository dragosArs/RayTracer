#pragma once
#include "Mesh.h"
#include <algorithm>

struct BVH {
	AABB boundingBox;
	int leftOffset;
	int rightOffset;
	std::unique_ptr<BVH> left = nullptr;
	std::unique_ptr<BVH> right = nullptr;
};

struct bvhNode {
	AABB boundingBox;
	int leftOffset;
	int rightOffset;
	int isLeaf;
};

