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
	/*
	std::cout << "Number of vertices: " << scene.vertices.size() << '\n';
	for (int i = 0; i < scene.vertices.size(); i++)
		std::cout << scene.vertices[i];
	
	std::cout << "Number of triangles: " << scene.triangles.size() << '\n';
	for(int i = 0; i < scene.triangles.size(); i++)
		std::cout << scene.triangles[i].vertexIndex0 << ", " << scene.triangles[i].vertexIndex1 << ", " <<  scene.triangles[i].vertexIndex2 << scene.triangles[i].materialIndex << '\n';
	*/

	std::cout << "Number of materials: " << scene.materials.size() << '\n';
	for (int i = 0; i < scene.materials.size(); i++)
		std::cout << scene.materials[i];
	
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
		triangles.push_back(Triangle{ index1, index2, index3, (uint32_t) mesh.material_ids[i / 3]});
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

