#include "Scene.h"
#include "../../Walnut/vendor/stb_image/stb_image.h"

//std::mutex coutMutex;

[[nodiscard]]
void Scene::load(const std::filesystem::path& objectFilePath, const std::filesystem::path& materialFilePath)
{
	auto objectFullFilePath = std::filesystem::current_path().string() + objectFilePath.string();
	auto materialFullFilePath = std::filesystem::current_path().string() + materialFilePath.string();
	rapidobj::MaterialLibrary ml = rapidobj::MaterialLibrary::SearchPath(materialFullFilePath, rapidobj::Load::Mandatory);
	rapidobj::Result result = rapidobj::ParseFile(objectFullFilePath, ml);

	if (result.error) 
		std::cout << result.error.code.message() << '\n';
	if(!rapidobj::Triangulate(result))
		std::cout << "Triangulation failed\n";

	loadMaterials(result.materials);
	for (const rapidobj::Shape& shape : result.shapes)
		createUniqueVertices(shape.mesh, result.attributes);
	std::cout << diffuseMaps.size() << '\n';
	std::cout << normalMaps.size() << '\n';
	AABB sceneBoundingBox;
	sceneBoundingBox.lower = glm::vec3{ FLT_MAX, FLT_MAX, FLT_MAX };
	sceneBoundingBox.upper = glm::vec3{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
	for (const Vertex& vertex : vertices)
	{
		sceneBoundingBox.lower.x = std::min(sceneBoundingBox.lower.x, vertex.position.x);
		sceneBoundingBox.lower.y = std::min(sceneBoundingBox.lower.y, vertex.position.y);
		sceneBoundingBox.lower.z = std::min(sceneBoundingBox.lower.z, vertex.position.z);

		sceneBoundingBox.upper.x = std::max(sceneBoundingBox.upper.x, vertex.position.x);
		sceneBoundingBox.upper.y = std::max(sceneBoundingBox.upper.y, vertex.position.y);
		sceneBoundingBox.upper.z = std::max(sceneBoundingBox.upper.z, vertex.position.z);
	}
	bvh = prepBvh(0, triangles.size(), sceneBoundingBox);
}

void Scene::loadMaterials(const std::vector<rapidobj::Material>& rapidObjMaterials)
{
	std::unordered_map<std::string, int> texMap;

	for (const rapidobj::Material& material : rapidObjMaterials)
	{
		Material myMaterial;
		//If a texture is specified in the material file, load it and add it to the scene
		if (material.diffuse_texname != "") {
			updateDiffuseMap(myMaterial, material.diffuse_texname, texMap);
		}
		if (material.normal_texname != "") {
			updateNormalMap(myMaterial, material.normal_texname, texMap);
		}
		myMaterial.kd = glm::vec3{ material.diffuse[0], material.diffuse[1], material.diffuse[2] };
		myMaterial.ks = glm::vec3{ material.specular[0], material.specular[1], material.specular[2] };
		myMaterial.shininess = material.shininess;
		myMaterial.transparency = 1 - material.dissolve;
		materials.push_back(myMaterial);
	}
}	

void Scene::updateDiffuseMap(Material& material, const std::string& diffuse_texname, std::unordered_map<std::string, int>& texMap)
{
	//If material hasn't been found before load it and add it to the scene, otherwise just assign the already loaded texture to the material
	if (texMap.find(diffuse_texname) == texMap.end()) {
		std::string filePathString = "assets\\textures\\" + diffuse_texname;
		const char* filePath = filePathString.c_str();
		int width, height, channels;
		std::shared_ptr<stbi_uc> imageData(stbi_load(filePath, &width, &height, &channels, 0), stbi_image_free);
		if (!imageData)
			std::cout << "Failed to load diffuse map\n";
		else {
			Texture texture;
			texture.pixels = loadTexture(imageData, width, height, channels);
			texture.width = width;
			texture.height = height;
			texture.channels = channels;
			texMap[diffuse_texname] = diffuseMaps.size();
			diffuseMaps.push_back(texture);
			material.diffuseMapIndex = diffuseMaps.size() - 1;
		}
	}
	else {
		material.diffuseMapIndex = texMap[diffuse_texname];
	}
}

void Scene::updateNormalMap(Material& material, const std::string& normal_texname, std::unordered_map<std::string, int>& texMap)
{
	//If material hasn't been found before load it and add it to the scene, otherwise just assign the already loaded texture to the material
	if (texMap.find(normal_texname) == texMap.end()) {
		std::string filePathString = "assets\\textures\\" + normal_texname;
		const char* filePath = filePathString.c_str();
		int width, height, channels;
		std::shared_ptr<stbi_uc> imageData(stbi_load(filePath, &width, &height, &channels, 0), stbi_image_free);
		if (!imageData || channels != 3)
			std::cout << "Failed to load normal map\n";
		else {
			Texture texture;
			texture.pixels = loadTexture(imageData, width, height, channels);
			texture.width = width;
			texture.height = height;
			texture.channels = channels;
			texMap[normal_texname] = normalMaps.size();
			normalMaps.push_back(texture);
			material.normalMapIndex = normalMaps.size() - 1;
		}
	}
	else {
		material.normalMapIndex = texMap[normal_texname];
	}
}


//"returns" a vector of triangles and vertices
void Scene::createUniqueVertices(const rapidobj::Mesh& mesh, const rapidobj::Attributes& attributes)
{
	std::unordered_map<Key, int> uniqueIndexKeys;

	for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
	{
		const Key key1 = { (uint32_t)mesh.indices[i].position_index, (uint32_t)mesh.indices[i].normal_index , (uint32_t)mesh.indices[i].texcoord_index };
		const Key key2 = { (uint32_t)mesh.indices[i + 1].position_index, (uint32_t)mesh.indices[i + 1].normal_index , (uint32_t)mesh.indices[i + 1].texcoord_index };
		const Key key3 = { (uint32_t)mesh.indices[i + 2].position_index, (uint32_t)mesh.indices[i + 2].normal_index , (uint32_t)mesh.indices[i + 2].texcoord_index };

		uint32_t index1 = getIndexOfVertex(key1, attributes, uniqueIndexKeys);
		uint32_t index2 = getIndexOfVertex(key2, attributes, uniqueIndexKeys);
		uint32_t index3 = getIndexOfVertex(key3, attributes, uniqueIndexKeys);

		triangles.push_back(Triangle{ index1, index2, index3, (uint32_t) mesh.material_ids[i / 3], getTriangleCentroid(vertices[index1].position, vertices[index2].position, vertices[index3].position)});
	}
}

uint32_t Scene::getIndexOfVertex(const Key& key, const rapidobj::Attributes& attributes, std::unordered_map<Key, int>& uniqueIndexKeys) {
	uint32_t index = 0;
	auto it = uniqueIndexKeys.find(key);

	if (it != uniqueIndexKeys.end())
		index = it->second;
	else
	{
		uniqueIndexKeys[key] = vertices.size();

		Vertex vertex;
		vertex.position = { attributes.positions[key.posIndex * 3], attributes.positions[key.posIndex * 3 + 1], attributes.positions[key.posIndex * 3 + 2] };

		if(key.normIndex != -1) 
			vertex.normal = { attributes.normals[key.normIndex * 3], attributes.normals[key.normIndex * 3 + 1], attributes.normals[key.normIndex * 3 + 2] };

		if (key.texIndex != -1) 
			vertex.texCoord = { attributes.texcoords[key.texIndex * 2], attributes.texcoords[key.texIndex * 2 + 1] };

		vertices.push_back(vertex);
		index = vertices.size() - 1;
	}

	return index;
}


std::unique_ptr<BVH> Scene::prepBvh(int left, int right, const AABB& box)
{
	//std::cout << "left: " << left << ", right: " << right << "\n";
	if (left == right - 1)
	{
		AABB leafAABB = createAABBForTriangle(triangles[left], vertices);
		return std::make_unique<BVH>(BVH{ leafAABB, left });//Leaf node
	}
	

	int split = 0;
	glm::vec3 boxSize = box.upper - box.lower;
	glm::vec3 splitPointRight = box.lower;
	glm::vec3 splitPointLeft = box.upper;
	
	if (boxSize.x >= boxSize.y && boxSize.x >= boxSize.z)
	{
		std::sort(triangles.begin() + left, triangles.begin() + right, [](const Triangle& lhs, const Triangle& rhs) {
			return lhs.centroid.x < rhs.centroid.x;
			});
		split = splitWithSAH(box, boxSize, splitPointLeft, splitPointRight, left, right, 0);
	}	
	else if (boxSize.y >= boxSize.z) 
	{
		std::sort(triangles.begin() + left, triangles.begin() + right, [](const Triangle& lhs, const Triangle& rhs) {
			return lhs.centroid.y < rhs.centroid.y;
			});
		split = splitWithSAH(box, boxSize, splitPointLeft, splitPointRight, left, right, 1);
	}
	else 
	{
		std::sort(triangles.begin() + left, triangles.begin() + right, [](const Triangle& lhs, const Triangle& rhs) {
			return lhs.centroid.z < rhs.centroid.z;
			});
		
		split = splitWithSAH(box, boxSize, splitPointLeft, splitPointRight, left, right, 2);
	}

	std::unique_ptr<BVH> leftBvh = std::move(prepBvh(left, split, AABB{ box.lower, splitPointLeft }));
	std::unique_ptr<BVH> rightBvh = std::move(prepBvh(split, right, AABB{ splitPointRight, box.upper }));
	AABB resBox = combineAABBs(leftBvh->boundingBox, rightBvh->boundingBox);
	return std::make_unique<BVH>(BVH{ resBox, -1, std::move(leftBvh), std::move(rightBvh) });
	
}

int Scene::splitWithSAH(const AABB& box, const glm::vec3& boxSize, glm::vec3& splitPointLeft, glm::vec3& splitPointRight, int left, int right, int axis)
{
	float traverse = box.lower[axis];
	float binSize = boxSize[axis] / 16;
	int iterate = left + 1;
	int split = left + 1;
	float minSah = FLT_MAX;
	float sah = 0;
	int otherAxis1 = (axis + 1) % 3;
	int otherAxis2 = (axis + 2) % 3;
	float P = boxSize[otherAxis1] * boxSize[otherAxis2];
	float S = boxSize[otherAxis1] + boxSize[otherAxis2];
	for (int i = 0; i < 16; ++i)
	{
		while (iterate < right - 1 && traverse >= triangles[iterate].centroid[axis])
			iterate++;
		sah = (P + S * (traverse - box.lower[axis])) * (iterate - left) 
			+ (P + S * (box.upper[axis] - traverse)) * (right - iterate);
		if (sah < minSah) 
		{
			minSah = sah;
			split = iterate;
			splitPointLeft[axis] = triangles[iterate].centroid[axis];
			splitPointRight[axis] = triangles[iterate].centroid[axis];
		}
		traverse += binSize;
	}

	return split;
}

AABB createAABBForTriangle(const Triangle& triangle, std::vector<Vertex>& vertices)
{
	Vertex v0 = vertices[triangle.vertexIndex0];
	Vertex v1 = vertices[triangle.vertexIndex1];
	Vertex v2 = vertices[triangle.vertexIndex2];

	float lowerX = std::min(v0.position.x, std::min(v1.position.x, v2.position.x));
	float lowerY = std::min(v0.position.y, std::min(v1.position.y, v2.position.y));
	float lowerZ = std::min(v0.position.z, std::min(v1.position.z, v2.position.z));

	float upperX = std::max(v0.position.x, std::max(v1.position.x, v2.position.x));
	float upperY = std::max(v0.position.y, std::max(v1.position.y, v2.position.y));
	float upperZ = std::max(v0.position.z, std::max(v1.position.z, v2.position.z));

	return AABB{ glm::vec3{lowerX, lowerY, lowerZ}, glm::vec3{upperX, upperY, upperZ} };
}

AABB combineAABBs(const AABB& box1, const AABB& box2)
{
	float lowerX = std::min(box1.lower.x, box2.lower.x);
	float lowerY = std::min(box1.lower.y, box2.lower.y);
	float lowerZ = std::min(box1.lower.z, box2.lower.z);

	float upperX = std::max(box1.upper.x, box2.upper.x);
	float upperY = std::max(box1.upper.y, box2.upper.y);
	float upperZ = std::max(box1.upper.z, box2.upper.z);

	return AABB{ glm::vec3{lowerX, lowerY, lowerZ}, glm::vec3{upperX, upperY, upperZ} };
}

std::vector<glm::vec3> loadTexture(std::shared_ptr<unsigned char> imageData, int width, int height, int numChannels)
{
	std::vector<glm::vec3> pixels;
	
	if (numChannels == 3)
	{
		uint32_t size = 3 * width * height;
		for (uint32_t i = 0; i < size; i += 3)
		{
			pixels.emplace_back(imageData.get()[i] / 255.0f, imageData.get()[i + 1] / 255.0f, imageData.get()[i + 2] / 255.0f);
		}
	}
	else if (numChannels == 1)
	{
		uint32_t size = width * height;
		for (uint32_t i = 0; i < size; i++)
		{
			pixels.emplace_back(imageData.get()[i] / 255.0f, imageData.get()[i] / 255.0f, imageData.get()[i] / 255.0f);
		}
	}
	else if (numChannels == 4)
	{
		uint32_t size = 4 * width * height;
		for (uint32_t i = 0; i < size; i += 4)
		{
			pixels.emplace_back(imageData.get()[i] / 255.0f, imageData.get()[i + 1] / 255.0f, imageData.get()[i + 2] / 255.0f);
		}
	}
	else if (numChannels == 2)
	{
		uint32_t size = 2 * width * height;
		for (uint32_t i = 0; i < size; i += 2)
		{
			pixels.emplace_back(imageData.get()[i] / 255.0f, imageData.get()[i] / 255.0f, imageData.get()[i] / 255.0f);
		}
	}
	else
	{
		std::cout << "Unsupported number of channels\n";
	}
	return pixels;

}

