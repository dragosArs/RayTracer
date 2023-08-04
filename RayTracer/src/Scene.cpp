#include "Scene.h"
#include "rapidobj.hpp"
[[nodiscard]] void loadScene(const std::filesystem::path& objectFilePath, const std::filesystem::path& materialFilePath, Scene& scene) {
	
	auto objectFullFilePath = std::filesystem::current_path().string() + objectFilePath.string();
	auto materialFullFilePath = std::filesystem::current_path().string() + materialFilePath.string();
	rapidobj::MaterialLibrary ml = rapidobj::MaterialLibrary::SearchPath(materialFullFilePath, rapidobj::Load::Mandatory);
	rapidobj::Result result = rapidobj::ParseFile(objectFullFilePath, ml);
	rapidobj::Triangulate(result);

	uint32_t facesCount = 0;
	for (int i = 0; i < result.attributes.positions.size(); i += 3) {
		float element1 = result.attributes.positions[i];
		float element2 = result.attributes.positions[i + 1];
		float element3 = result.attributes.positions[i + 2];
		scene.positions.push_back(glm::vec3(element1, element2, element3));
	}

	for (int i = 0; i < result.attributes.normals.size(); i += 2) {
		float element1 = result.attributes.normals[i];
		float element2 = result.attributes.normals[i + 1];
		float element3 = result.attributes.normals[i + 2];
		scene.normals.push_back(glm::vec3(element1, element2, element3));
	}

	for (int i = 0; i < result.attributes.texcoords.size(); i += 2) {
		float element1 = result.attributes.texcoords[i];
		float element2 = result.attributes.texcoords[i + 1];
		scene.texCoords.push_back(glm::vec2(element1, element2));
	}

	for (int i = 0; i < result.materials.size(); i += 1) {
		Material material;
		material.kd = glm::vec3(result.materials[i].diffuse[0], result.materials[i].diffuse[1], result.materials[i].diffuse[2]);
		material.ks = glm::vec3(result.materials[i].specular[0], result.materials[i].specular[1], result.materials[i].specular[2]);
		material.shininess = result.materials[i].shininess;
		material.transparency = result.materials[i].dissolve;
		scene.materials.push_back(material);
	}

	//std::cout << "Number of indices " << shape.mesh.indices.size() << std::endl;
	//std::cout << "Number of faces : " << shape.mesh.num_face_vertices.size() << std::endl;
	for (const rapidobj::Shape& shape: result.shapes) {
		//std::cout << "Number of indices " << shape.mesh.indices.size() << std::endl;
		uint32_t j = 0;
		for (uint32_t i = 0; i < shape.mesh.num_face_vertices.size(); i++) {
			//std::cout << "Number of faces : " << (int) shape.mesh.num_face_vertices[i] << std::endl;
			//if (shape.mesh.num_face_vertices[i] == 3) {
				Triangle triangle;
				triangle.materialIndex = shape.mesh.material_ids[i];
				Index vertex0;
				vertex0.positionIndex = shape.mesh.indices[j].position_index;
				vertex0.normalIndex = shape.mesh.indices[j].normal_index;
				vertex0.texCoordIndex = shape.mesh.indices[j].texcoord_index;
				vertex0.materialIndex = shape.mesh.material_ids[i];
				triangle.vertex0 = vertex0;

				Index vertex1;
				vertex1.positionIndex = shape.mesh.indices[j + 1].position_index;
				vertex1.normalIndex = shape.mesh.indices[j + 1].normal_index;
				vertex1.texCoordIndex = shape.mesh.indices[j + 1].texcoord_index;
				vertex1.materialIndex = shape.mesh.material_ids[i];
				triangle.vertex1 = vertex1;
				//std::cout << "{";
				Index vertex2;
				vertex2.positionIndex = shape.mesh.indices[j + 2].position_index;
				vertex2.normalIndex = shape.mesh.indices[j + 2].normal_index;
				vertex2.texCoordIndex = shape.mesh.indices[j + 2].texcoord_index;
				vertex2.materialIndex = shape.mesh.material_ids[i];
				triangle.vertex2 = vertex2;
				
				scene.triangles.push_back(triangle);
				//std::cout << "}" << std::endl;
			//}
			j += shape.mesh.num_face_vertices[i];
		}
	}
	//std::cout << "a" << std::endl;
}