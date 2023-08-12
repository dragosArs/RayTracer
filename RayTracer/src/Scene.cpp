#include "Scene.h"

[[nodiscard]]
void loadScene(const std::filesystem::path& objectFilePath, const std::filesystem::path& materialFilePath, Scene& scene) {
	
	auto objectFullFilePath = std::filesystem::current_path().string() + objectFilePath.string();
	auto materialFullFilePath = std::filesystem::current_path().string() + materialFilePath.string();
	rapidobj::MaterialLibrary ml = rapidobj::MaterialLibrary::SearchPath(materialFullFilePath, rapidobj::Load::Mandatory);
	rapidobj::Result result = rapidobj::ParseFile(objectFullFilePath, ml);
	if (result.error) {
		std::cout << result.error.code.message() << '\n';
	}
	if(!rapidobj::Triangulate(result))
		std::cout << "Triangulation failed\n";


	for (const rapidobj::Material& material : result.materials) {
		Material myMaterial;
		myMaterial.kd = glm::vec3{ material.diffuse[0], material.diffuse[1], material.diffuse[2]};
		myMaterial.ks = glm::vec3{ material.specular[0], material.specular[1], material.specular[2]};
		myMaterial.shininess = material.shininess;
		myMaterial.transparency = 1 - material.dissolve;
		scene.materials.push_back(myMaterial);
	}
	int j = 0;
	for (const rapidobj::Shape& shape : result.shapes) {
		
		createUniqueVertices(shape.mesh, result.attributes, scene.triangles, scene.vertices);
	}
	
	scene.bvh = prepBvh(scene.vertices, scene.triangles, 0, scene.triangles.size() - 1, 0);

	//std::cout << scene.bvh.left->boundingBox.lower.x << " " << scene.bvh.left->boundingBox.lower.y << " " << scene.bvh.left->boundingBox.lower.z << "\n";
	//std::cout << scene.bvh.boundingBox.upper.x << " " << scene.bvh.boundingBox.upper.y << " " << scene.bvh.boundingBox.upper.z << "\n";
	
}


//"returns" a vector of triangles and vertices
void createUniqueVertices(const rapidobj::Mesh& mesh, const rapidobj::Attributes& attributes, std::vector<Triangle>& triangles, std::vector<Vertex>& vertices) {
	std::unordered_map<Key, int> uniqueIndexKeys;

	for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
		const Key key1 = { (uint32_t)mesh.indices[i].position_index, (uint32_t)mesh.indices[i].normal_index , (uint32_t)mesh.indices[i].texcoord_index };
		const Key key2 = { (uint32_t)mesh.indices[i + 1].position_index, (uint32_t)mesh.indices[i + 1].normal_index , (uint32_t)mesh.indices[i + 1].texcoord_index };
		const Key key3 = { (uint32_t)mesh.indices[i + 2].position_index, (uint32_t)mesh.indices[i + 2].normal_index , (uint32_t)mesh.indices[i + 2].texcoord_index };

		uint32_t index1 = getIndexOfVertex(key1, attributes, vertices, uniqueIndexKeys);
		uint32_t index2 = getIndexOfVertex(key2, attributes, vertices, uniqueIndexKeys);
		uint32_t index3 = getIndexOfVertex(key3, attributes, vertices, uniqueIndexKeys);

		triangles.push_back(Triangle{ index1, index2, index3, (uint32_t) mesh.material_ids[i / 3], getTriangleCentroid(vertices[index1].position, vertices[index2].position, vertices[index3].position)});
	}
}

uint32_t getIndexOfVertex(const Key& key, const rapidobj::Attributes& attributes, std::vector<Vertex>& vertices, std::unordered_map<Key, int>& uniqueIndexKeys) {
	uint32_t index = 0;
	auto it = uniqueIndexKeys.find(key);
	if (it != uniqueIndexKeys.end()) {
		index = it->second;
	}
	else {
		uniqueIndexKeys[key] = vertices.size();
		glm::vec3 pos = { attributes.positions[key.posIndex * 3], attributes.positions[key.posIndex * 3 + 1], attributes.positions[key.posIndex * 3 + 2] };
		glm::vec3 norm;
		glm::vec2 tex;
		if(key.normIndex != -1) 
			norm = { attributes.normals[key.normIndex * 3], attributes.normals[key.normIndex * 3 + 1], attributes.normals[key.normIndex * 3 + 2] };
		else 
			norm = { 0, 0, 0 };
		if (key.texIndex != -1) 
			tex = { attributes.texcoords[key.texIndex * 2], attributes.texcoords[key.texIndex * 2 + 1] };
		else
			tex = { 0, 0 };
		Vertex vertex = { pos, norm, tex };
		vertices.push_back(vertex);
		index = vertices.size() - 1;

	}
	return index;
}

std::unique_ptr<BVH> prepBvh(const std::vector<Vertex>& vertices, std::vector<Triangle>& triangles, int left, int right, int level) {
	if (level == 0)
		std::sort(triangles.begin() + left, triangles.begin() + right, [](const Triangle& lhs, const Triangle& rhs) {
			return lhs.centroid.z < rhs.centroid.z;
		});
	else if(level == 1) 
		std::sort(triangles.begin() + left, triangles.begin() + right, [](const Triangle& lhs, const Triangle& rhs) {
		return lhs.centroid.x < rhs.centroid.x;
	});
	else if (level == 2)
		std::sort(triangles.begin() + left, triangles.begin() + right, [](const Triangle& lhs, const Triangle& rhs) {
			return lhs.centroid.y < rhs.centroid.y;
	});

	int mid = (left + right) / 2;
	if (left != right) {
		//std::cout << left << ", " << mid << ", " << right << std::endl;
		std::unique_ptr<BVH> leftBvh = std::move(prepBvh(vertices, triangles, left, mid, (level + 1) % 3));
		std::unique_ptr<BVH> rightBvh = std::move(prepBvh(vertices, triangles, mid + 1, right, (level + 1) % 3));
		
		float minX = std::min(leftBvh->boundingBox.lower.x, rightBvh->boundingBox.lower.x);
		float minY = std::min(leftBvh->boundingBox.lower.y, rightBvh->boundingBox.lower.y);
		float minZ = std::min(leftBvh->boundingBox.lower.z, rightBvh->boundingBox.lower.z);

		float maxX = std::max(leftBvh->boundingBox.upper.x, rightBvh->boundingBox.upper.x);
		float maxY = std::max(leftBvh->boundingBox.upper.y, rightBvh->boundingBox.upper.y);
		float maxZ = std::max(leftBvh->boundingBox.upper.z, rightBvh->boundingBox.upper.z);
		AABB mergedBox = AABB{ glm::vec3{minX, minY, minZ}, glm::vec3{maxX, maxY, maxZ} };
		
		//std::cout << leftBvh.triangleIndex << ", " << rightBvh.triangleIndex << std::endl;
		std::unique_ptr<BVH> bvh(new BVH{ mergedBox, -1 });
		bvh->left = std::move(leftBvh);
		bvh->right = std::move(rightBvh);
		return bvh;
	}
	else {
		Triangle& triangle = triangles[left];
		float minX = std::min(vertices[triangle.vertexIndex0].position.x, std::min(vertices[triangle.vertexIndex1].position.x, vertices[triangle.vertexIndex2].position.x));
		float minY = std::min(vertices[triangle.vertexIndex0].position.y, std::min(vertices[triangle.vertexIndex1].position.y, vertices[triangle.vertexIndex2].position.y));
		float minZ = std::min(vertices[triangle.vertexIndex0].position.z, std::min(vertices[triangle.vertexIndex1].position.z, vertices[triangle.vertexIndex2].position.z));
		
		float maxX = std::max(vertices[triangle.vertexIndex0].position.x, std::max(vertices[triangle.vertexIndex1].position.x, vertices[triangle.vertexIndex2].position.x));
		float maxY = std::max(vertices[triangle.vertexIndex0].position.y, std::max(vertices[triangle.vertexIndex1].position.y, vertices[triangle.vertexIndex2].position.y));
		float maxZ = std::max(vertices[triangle.vertexIndex0].position.z, std::max(vertices[triangle.vertexIndex1].position.z, vertices[triangle.vertexIndex2].position.z));
		//std::cout << left << std::endl;
		AABB mergedBox = { glm::vec3{minX, minY, minZ}, glm::vec3{maxX, maxY, maxZ} };
		std::cout << mergedBox.lower.x << ", " << mergedBox.lower.y << ", " << mergedBox.lower.z << ", " << mergedBox.upper.x << ", " << mergedBox.upper.y << ", " << mergedBox.upper.z << std::endl;
		std::unique_ptr<BVH> bvh(new BVH{ mergedBox, left });
		return bvh;
	}
}

