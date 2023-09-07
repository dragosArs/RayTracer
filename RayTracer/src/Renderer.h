#pragma once

#include "Walnut/Image.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>
#include <glm/glm.hpp>


struct Coord {
	int x;
	int y;
	/*
	bool operator==(const Key& other) const {
		return (posIndex == other.posIndex && normIndex == other.normIndex && texIndex == other.texIndex);
	}
	*/
};

inline bool operator==(const Coord& lhs, const Coord& rhs) {
	return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

template <>
struct std::hash<Coord> {
	std::size_t operator()(const Coord& ti) const {
		// Combine the hash of the three integers using a simple hash function
		return std::hash<int>()(ti.x) ^ (std::hash<int>()(ti.y) << 1);
	}
};

class Renderer
{
public:
	struct Settings
	{
		bool enableRayTracing = true;
		bool Accumulate = true;
		bool applyBilinearInterpolation = false;
	};

	struct VisualDebugging
	{
		bool enableWireframeTriangles = false;
		bool enableWireframeBvh = false;
	};
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera, const Settings& settings, const VisualDebugging& visDebugging);
	void Debug(const Scene& scene, const Camera& camera);
	void RasterizeLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color, std::unordered_map<Coord, float>& zBuffer);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_finalImage; }
	Settings& GetSettings() { return m_settings; }
	VisualDebugging& GetVisualDebugging() { return m_visualDebugging; }
private:

	glm::vec3 perPixel(uint32_t x, uint32_t y, bool debug); // RayGen
	void traceRay(Ray& ray, BasicHitInfo& hitInfo, bool debug);
	bool isInShadow(const Ray& ray, float length, bool debug, uint32_t originalTriangleIndex);
	FullHitInfo retrieveFullHitInfo(const Scene* scene, const BasicHitInfo& basicHitInfo, const Ray& ray);

private:
	std::shared_ptr<Walnut::Image> m_finalImage;
	Settings m_settings;
	VisualDebugging m_visualDebugging;

	std::vector<uint32_t> m_imageHorizontalIter, m_imageVerticalIter;
	const Scene* m_activeScene = nullptr;
	const Camera* m_activeCamera = nullptr;
	uint32_t* m_imageData = nullptr;
};