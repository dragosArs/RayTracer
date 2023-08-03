#pragma once
//#include "image.h"
// Suppress warnings in third-party code.
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord; // Texture coordinate
};

/*
struct Material {
	glm::vec3 kd; // Diffuse color.
	glm::vec3 ks{ 0.0f };
	float shininess{ 1.0f };
	float transparency{ 1.0f };
};
*/

struct Material
{
	glm::vec3 Albedo{ 1.0f };
	glm::vec3 EmissionColor{ 0.0f };
	float Roughness = 1.0f;
	float Metallic = 0.0f;
	float EmissionPower = 0.0f;
	float GetShininess() const { return (1.0f - Roughness) * (1.0f - Roughness); }
	glm::vec3 GetDiffuse() const { return Albedo * (1.0f - Metallic); }
	glm::vec3 GetSpecular() const { return Metallic * (1.0f - Roughness) * Albedo; }
	glm::vec3 GetEmission() const { return EmissionColor * EmissionPower; }
	//glm::vec3 GetSpecular() const { return glm::mix(glm::vec3(0.04f), Albedo, Metallic) * glm::mix(0.08f, 1.0f, Roughness); }
	// Optional texture that replaces kd; use as follows:
	// 
	// if (material.kdTexture) {
	//   material.kdTexture->getTexel(...);
	// }
	//std::optional<Image> kdTexture;
};

struct Mesh {
	// Vertices contain the vertex positions and normals of the mesh.
	std::vector<Vertex> vertices;
	// A triangle contains a triplet of values corresponding to the indices of the 3 vertices in the vertices array.
	std::vector<glm::uvec3> triangles;
	Material material;
};

[[nodiscard]] std::vector<Mesh> loadMesh(const std::filesystem::path& file, bool normalize = false, bool postProcess = true);