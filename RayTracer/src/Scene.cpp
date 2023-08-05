#include "Scene.h"




struct Tuple {
	uint32_t posIndex;
	uint32_t normIndex;
	uint32_t texIndex;
	/*
	bool operator==(const Tuple& other) const {
		return (posIndex == other.posIndex && normIndex == other.normIndex && texIndex == other.texIndex);
	}
	*/
};

inline bool operator==(const Tuple& lhs, const Tuple& rhs) {
	return (lhs.posIndex == rhs.posIndex && lhs.normIndex == rhs.normIndex && lhs.texIndex == rhs.texIndex);
}

template <>
struct std::hash<Tuple> {
	std::size_t operator()(const Tuple& ti) const {
		// Combine the hash of the three integers using a simple hash function
		return std::hash<int>()(ti.posIndex) ^ (std::hash<int>()(ti.normIndex) << 1) ^ (std::hash<int>()(ti.texIndex) << 2);
	}
};



[[nodiscard]]
void loadScene(const std::filesystem::path& objectFilePath, const std::filesystem::path& materialFilePath, Scene& scene) {
	
	auto objectFullFilePath = std::filesystem::current_path().string() + objectFilePath.string();
	auto materialFullFilePath = std::filesystem::current_path().string() + materialFilePath.string();
	rapidobj::MaterialLibrary ml = rapidobj::MaterialLibrary::SearchPath(materialFullFilePath, rapidobj::Load::Mandatory);
	rapidobj::Result result = rapidobj::ParseFile(objectFullFilePath, ml);
	if (result.error) {
		std::cout << result.error.code.message() << '\n';
	}
	
	rapidobj::Triangulate(result);

	std::vector<Triangle> triangles;
	std::vector<Vertex> vertices;
	int j = 0;
	for (const rapidobj::Shape& shape : result.shapes) {
		createUniqueVertices(shape.mesh, result.attributes, triangles, vertices);
		scene.meshes.push_back({j, vertices, triangles });
		j++;
	}
}


//"returns" a vector of triangles and vertices
[[nodiscard]]
void createUniqueVertices(const rapidobj::Mesh& mesh, const rapidobj::Attributes& attributes, std::vector<Triangle> triangles, std::vector<Vertex> vertices) {
	std::unordered_map<Tuple, int> uniqueIndexTuples;

	for (uint32_t i = 0; i < mesh.indices.size(); i += 3) {
		Tuple tuple1 = {mesh.indices[i].position_index, mesh.indices[i].normal_index, mesh.indices[i].texcoord_index};
		uint32_t index1 = 0;
		auto it1 = std::find(uniqueIndexTuples.begin(), uniqueIndexTuples.end(), tuple1);
		if(it1 != uniqueIndexTuples.end()) {
			index1 = it1->second;
		}
		else {
			uniqueIndexTuples[tuple1] = vertices.size();
			glm::vec3 pos = { attributes.positions[tuple1.posIndex], attributes.positions[tuple1.posIndex + 1], attributes.positions[tuple1.posIndex + 2] };
			glm::vec3 norm = { attributes.normals[tuple1.normIndex], attributes.normals[tuple1.normIndex + 1], attributes.normals[tuple1.normIndex + 2] };
			glm::vec2 tex = { attributes.texcoords[tuple1.texIndex], attributes.texcoords[tuple1.texIndex + 1] };
			Vertex vertex = { pos, norm, tex };
			vertices.push_back(vertex);
			index1 = vertices.size() - 1;
			
		}

		Tuple tuple2 = { mesh.indices[i + 1].position_index, mesh.indices[i + 1].normal_index, mesh.indices[i + 1].texcoord_index };
		uint32_t index2 = 0;
		auto it2 = std::find(uniqueIndexTuples.begin(), uniqueIndexTuples.end(), tuple2);
		if (it2 != uniqueIndexTuples.end()) {
			index2 = it2->second;
		}
		else {
			uniqueIndexTuples[tuple2] = vertices.size();
			glm::vec3 pos = { attributes.positions[tuple2.posIndex], attributes.positions[tuple2.posIndex + 1], attributes.positions[tuple2.posIndex + 2] };
			glm::vec3 norm = { attributes.normals[tuple2.normIndex], attributes.normals[tuple2.normIndex + 1], attributes.normals[tuple2.normIndex + 2] };
			glm::vec2 tex = { attributes.texcoords[tuple2.texIndex], attributes.texcoords[tuple2.texIndex + 1] };
			Vertex vertex = { pos, norm, tex };
			vertices.push_back(vertex);
			index2 = vertices.size() - 1;

		}

		Tuple tuple3 = { mesh.indices[i + 2].position_index, mesh.indices[i + 2].normal_index, mesh.indices[i + 2].texcoord_index };
		uint32_t index3 = 0;
		auto it3 = std::find(uniqueIndexTuples.begin(), uniqueIndexTuples.end(), tuple3);
		if (it3 != uniqueIndexTuples.end()) {
			index3 = it3->second;
		}
		else {
			uniqueIndexTuples[tuple3] = vertices.size();
			glm::vec3 pos = { attributes.positions[tuple3.posIndex], attributes.positions[tuple3.posIndex + 1], attributes.positions[tuple3.posIndex + 2] };
			glm::vec3 norm = { attributes.normals[tuple3.normIndex], attributes.normals[tuple3.normIndex + 1], attributes.normals[tuple3.normIndex + 2] };
			glm::vec2 tex = { attributes.texcoords[tuple3.texIndex], attributes.texcoords[tuple3.texIndex + 1] };
			Vertex vertex = { pos, norm, tex };
			vertices.push_back(vertex);
			index3 = vertices.size() - 1;

		}
		//this has to be done when all three vertices are processed
		triangles.push_back(Triangle{ index1, index2, index3, (uint32_t) mesh.material_ids[i / 3]});
	}


}