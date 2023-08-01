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

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

	void ResetFrameIndex() { m_FrameIndex = 1; }
	Settings& GetSettings() { return m_Settings; }
private:
	struct HitInfo
	{
		float HitDistance = std::numeric_limits<float>::infinity();;
		glm::vec3 HitPos = glm::vec3{ 0.0f };
		glm::vec3 HitNormal = glm::vec3{ 0.0f };

		int ObjectIndex = 0;
	};

	glm::vec4 PerPixel(uint32_t x, uint32_t y); // RayGen
	HitInfo TraceRay(const Ray& ray);
	HitInfo ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	//glm::vec4 Miss(const Ray& ray); // Background
	bool intersectRayWithPlane(const Plane& plane, Ray& ray);
	bool pointInTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& n, const glm::vec3& p);
	bool intersectRayWithTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, Ray& ray, HitInfo& hitInfo);
	bool intersectRayWithShape(const Sphere& sphere, Ray& ray, HitInfo& hitInfo);
	float intersectRayWithShape(const AxisAlignedBox& box, Ray& ray);

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	Settings m_Settings;

	std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	uint32_t* m_ImageData = nullptr;
	glm::vec4* m_AccumulationData = nullptr;

	uint32_t m_FrameIndex = 1;
};