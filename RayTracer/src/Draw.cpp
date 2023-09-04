#include "Draw.h"
#include <glm/fwd.hpp>
#include <vector>
#include <queue>


std::vector<std::pair<glm::vec3, glm::vec3>> DrawBvh(const BVH* bvh)
{

	std::vector<std::pair<glm::vec3, glm::vec3>> toReturn;
	std::queue<const BVH*> queue;
	queue.push(bvh);

	while (queue.size() > 0)
	{
		const BVH* cur = queue.front();
		queue.pop();
		BVH* leftBvh = cur->left.get();
		BVH* rightBvh = cur->right.get();

		if (leftBvh->triangleIndex == -1)
			queue.push(leftBvh);

		if (rightBvh->triangleIndex == -1)
			queue.push(rightBvh);
	}

	return toReturn;
}

std::vector<std::pair<glm::vec3, glm::vec3>> DrawMeshes(const Scene& scene)
{
	std::vector<std::pair<glm::vec3, glm::vec3>> toReturn;
	
	for (const Triangle& triangle : scene.triangles)
	{
		toReturn.push_back(std::pair<glm::vec3, glm::vec3> (scene.vertices[triangle.vertexIndex0].position, scene.vertices[triangle.vertexIndex1].position));
		toReturn.push_back(std::pair<glm::vec3, glm::vec3>(scene.vertices[triangle.vertexIndex1].position, scene.vertices[triangle.vertexIndex2].position));
		toReturn.push_back(std::pair<glm::vec3, glm::vec3>(scene.vertices[triangle.vertexIndex2].position, scene.vertices[triangle.vertexIndex0].position));
	}

	return toReturn;

}