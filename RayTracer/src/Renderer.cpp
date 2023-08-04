#include "Renderer.h"
#include "Shading.h"
#include "Calculation_utility.h"

#include "Walnut/Random.h"

#include <execution>
#include <iostream>

namespace Utils {

	static uint32_t ConvertToRGBA(const glm::vec3& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = 255;

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}

}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_finalImage)
	{
		// No resize necessary
		if (m_finalImage->GetWidth() == width && m_finalImage->GetHeight() == height)
			return;

		m_finalImage->Resize(width, height);
	}
	else
	{
		m_finalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_imageData;
	m_imageData = new uint32_t[width * height];

	m_imageHorizontalIter.resize(width);
	m_imageVerticalIter.resize(height);
	for (uint32_t i = 0; i < width; i++)
		m_imageHorizontalIter[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_imageVerticalIter[i] = i;
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_activeScene = &scene;
	m_activeCamera = &camera;
#define MT 0
#if DEBUG
	for (uint32_t y = 0; y < m_finalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_finalImage->GetWidth(); x++)
		{
			m_imageData[x + y * m_finalImage->GetWidth()] = Utils::ConvertToRGBA(perPixel(x, y, false));
			//m_imageData[x + y * m_finalImage->GetWidth()] = Utils::ConvertToRGBA(glm::vec3{0.1f, 0.2f, 0.9f});
		}
	}
	

#else
	std::for_each(std::execution::par, m_imageVerticalIter.begin(), m_imageVerticalIter.end(),
		[this](uint32_t y)
		{
			std::for_each(std::execution::par, m_imageHorizontalIter.begin(), m_imageHorizontalIter.end(),
			[this, y](uint32_t x)
				{
					m_imageData[x + y * m_finalImage->GetWidth()] = Utils::ConvertToRGBA(perPixel(x, y, false));
				});
		});

#endif

	m_finalImage->SetData(m_imageData);
}

void Renderer::Debug(const Scene& scene, const Camera& camera)
{
	m_activeScene = &scene;
	m_activeCamera = &camera;
	glm::vec3 finalColor = perPixel(camera.xDebug, camera.yDebug, true);
	std::cout << "finalColor: " << finalColor.x << " " << finalColor.y << " " << finalColor.z << std::endl << std::endl;
}


glm::vec3 Renderer::perPixel(uint32_t x, uint32_t y, bool debug)
{

	Ray ray;
	Ray shadowRay;
	ray.origin = m_activeCamera->GetPosition();
	ray.direction = m_activeCamera->GetRayDirections()[x + y * m_finalImage->GetWidth()];
	//std::cout<<glm::length(ray.direction)<<std::endl;
	//std::cout <<m_activeScene->triangles.size()<<std::endl;
	glm::vec3 color = glm::vec3{ 0.1f };//ambient light
	glm::vec3 reflectiveContribution = glm::vec3{ 1.0f };
	HitInfo hitInfo;
	int bounces = 1;
	for (int i = 0; i < bounces; i++) {
		//"reset" ray
		ray.t = -1;
		if (i >= 1)
			reflectiveContribution = hitInfo.material.ks * reflectiveContribution;
		traceRay(ray, hitInfo);
		
		if (ray.t > 0) {
			//std::cout << ray.t << std::endl;
			ray.direction = glm::reflect(ray.direction, hitInfo.normal);
			//very important to avoid self-intersection by using an offset
			ray.origin = hitInfo.position + 0.0001f * ray.direction;


			for (const PointLight& pointLight : m_activeScene->lightSources) {
				shadowRay.direction = glm::normalize(pointLight.position - hitInfo.position);
				shadowRay.origin = hitInfo.position + 0.0001f * shadowRay.direction;
				//reflective component should contribute proportionally to the specular component of the material it reflects off of
				if (!isInShadow(shadowRay))
					color += reflectiveContribution * phongFull(hitInfo, *m_activeCamera, pointLight);
			}
		}
		else
			i = bounces;
		
	}
	glm::vec3 finalColor = glm::clamp(color, glm::vec3{ 0.0f }, glm::vec3{ 1.0f });
	return finalColor;
}

//This function doesn't return anything, but changes ray.t and hitInfo
void Renderer::traceRay(Ray& ray, HitInfo& hitInfo)
{
	for (const Sphere& sphere: m_activeScene->spheres)
	{
		intersectSphere(ray, sphere, hitInfo);
	}

	for (const Triangle& triangle : m_activeScene->triangles)
	{
		intersectTriangle(ray, hitInfo, *m_activeScene, triangle);
	}	

}


bool Renderer::isInShadow(const Ray& ray) {

	for (const Sphere& sphere : m_activeScene->spheres)
	{
		if (intersectSphere(ray, sphere)) {
			return true;
		}
	}

	return false;
}

