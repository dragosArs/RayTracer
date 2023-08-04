#include "Mesh.h"

[[nodiscard]] std::vector<Mesh> loadMeshes(const std::filesystem::path& file) {
	std::vector<Mesh> meshes;
	rapidobj::Result result = rapidobj::ParseFile(file.string().c_str());
	rapidobj::Triangulate(result);

		return meshes;
}