#pragma once

#include "Walnut/Image.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>
#include <glm/glm.hpp>

class Renderer
{
public:
	struct Settings
	{
		bool Accumulate = true;
	};
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_finalImage; }
	Settings& GetSettings() { return m_settings; }
private:

	glm::vec3 perPixel(uint32_t x, uint32_t y); // RayGen
	void traceRay(Ray& ray, HitInfo& hitInfo);
	bool isInShadow(const Ray& ray);
	bool intersectRayWithPlane(const Plane& plane, Ray& ray);
	bool pointInTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& n, const glm::vec3& p);
	bool intersectRayWithTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, Ray& ray, HitInfo& hitInfo);
	bool intersectRayWithShape(const Sphere& sphere, Ray& ray, HitInfo& hitInfo);
	float intersectRayWithShape(const AxisAlignedBox& box, Ray& ray);

private:
	std::shared_ptr<Walnut::Image> m_finalImage;
	Settings m_settings;

	std::vector<uint32_t> m_imageHorizontalIter, m_imageVerticalIter;
	const Scene* m_activeScene = nullptr;
	const Camera* m_activeCamera = nullptr;
	uint32_t* m_imageData = nullptr;
};