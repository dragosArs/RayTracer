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
		glm::vec3 vertices[8] = {
		glm::vec3(cur->boundingBox.lower.x, cur->boundingBox.lower.y, cur->boundingBox.lower.z),
		glm::vec3(cur->boundingBox.upper.x, cur->boundingBox.lower.y, cur->boundingBox.lower.z),
		glm::vec3(cur->boundingBox.lower.x, cur->boundingBox.upper.y, cur->boundingBox.lower.z),
		glm::vec3(cur->boundingBox.upper.x, cur->boundingBox.upper.y, cur->boundingBox.lower.z),
		glm::vec3(cur->boundingBox.lower.x, cur->boundingBox.lower.y, cur->boundingBox.upper.z),
		glm::vec3(cur->boundingBox.upper.x, cur->boundingBox.lower.y, cur->boundingBox.upper.z),
		glm::vec3(cur->boundingBox.lower.x, cur->boundingBox.upper.y, cur->boundingBox.upper.z),
		glm::vec3(cur->boundingBox.upper.x, cur->boundingBox.upper.y, cur->boundingBox.upper.z)
		};

		// Define the edges using pairs of vertices
		int edgeIndices[12][2] = {
			{0, 1}, {1, 3}, {3, 2}, {2, 0}, // Front face
			{4, 5}, {5, 7}, {7, 6}, {6, 4}, // Back face
			{0, 4}, {1, 5}, {2, 6}, {3, 7}  // Connecting edges
		};

		for (int i = 0; i < 12; ++i) {
			toReturn.push_back(std::make_pair(vertices[edgeIndices[i][0]], vertices[edgeIndices[i][1]]));
		}
		BVH* leftBvh = cur->left.get();
		BVH* rightBvh = cur->right.get();

		if (leftBvh != nullptr)
			queue.push(leftBvh);

		if (rightBvh != nullptr)
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