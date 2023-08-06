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
	void Debug(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_finalImage; }
	Settings& GetSettings() { return m_settings; }
private:

	glm::vec3 perPixel(uint32_t x, uint32_t y, bool debug); // RayGen
	void traceRay(Ray& ray, BasicHitInfo& hitInfo);
	bool isInShadow(const Ray& ray);
	FullHitInfo retrieveFullHitInfo(const Scene* scene, const BasicHitInfo& basicHitInfo, const Ray& ray);

private:
	std::shared_ptr<Walnut::Image> m_finalImage;
	Settings m_settings;

	std::vector<uint32_t> m_imageHorizontalIter, m_imageVerticalIter;
	const Scene* m_activeScene = nullptr;
	const Camera* m_activeCamera = nullptr;
	uint32_t* m_imageData = nullptr;
};