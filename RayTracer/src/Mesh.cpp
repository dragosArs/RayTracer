#include "Mesh.h"

std::ostream& operator<<(std::ostream& os, const Vertex& v) {
	os << "Position: " << v.position.x << ", " << v.position.y << ", " << v.position.z << "\n";
	os << "Normal: " << v.normal.x << ", " << v.normal.y << ", " << v.normal.z << "\n";
	os << "TexCoords: " << v.texCoord.x << ", " << v.texCoord.y << "\n" << "\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const Material& m) {
	os << "Diffuse component: " << m.kd.x << ", " << m.kd.y << ", " << m.kd.z << "\n";
	os << "Specular: " << m.ks.x << ", " << m.ks.y << ", " << m.ks.z << "\n";
	os << "Shininess: " << m.shininess << "\n";
	os << "Transparency: " << m.transparency << "\n" << "\n";
	return os;
}



glm::vec3 getTriangleCentroid(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3) {
	return glm::vec3(pos1.x + pos2.x + pos3.x, pos1.y + pos2.y + pos3.y, pos1.z + pos2.z + pos3.z) / 3.0f;
}