#include "Mesh.h"

std::ostream& operator<<(std::ostream& os, const Vertex& v) {
	os << "Position: " << v.position.x << ", " << v.position.y << ", " << v.position.z << std::endl;
	os << "Normal: " << v.normal.x << ", " << v.normal.y << ", " << v.normal.z << std::endl;
	os << "TexCoords: " << v.texCoord.x << ", " << v.texCoord.y << std::endl << std::endl;
	return os;
}

std::ostream& operator<<(std::ostream& os, const Material& m) {
	os << "Diffuse component: " << m.kd.x << ", " << m.kd.y << ", " << m.kd.z << std::endl;
	os << "Specular: " << m.ks.x << ", " << m.ks.y << ", " << m.ks.z << std::endl;
	os << "Shininess: " << m.shininess << std::endl;
	os << "Transparency: " << m.transparency << std::endl << std::endl;
	return os;
}



glm::vec3 getTriangleCentroid(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3) {
	return glm::vec3(pos1.x + pos2.x + pos3.x / 3.0f, pos1.y + pos2.y + pos3.y / 3.0f, pos1.z + pos2.z + pos3.z) / 3.0f;
}