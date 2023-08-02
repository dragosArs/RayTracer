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
		uint8_t a = 255.0f;

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

#define MT 1
#if MT
	std::for_each(std::execution::par, m_imageVerticalIter.begin(), m_imageVerticalIter.end(),
		[this](uint32_t y)
		{
			std::for_each(std::execution::par, m_imageHorizontalIter.begin(), m_imageHorizontalIter.end(),
			[this, y](uint32_t x)
				{
					m_imageData[x + y * m_finalImage->GetWidth()] = Utils::ConvertToRGBA(perPixel(x, y));
				});
		});

#else

	for (uint32_t y = 0; y < m_finalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_finalImage->GetWidth(); x++)
		{
			m_imageData[x + y * m_finalImage->GetWidth()] = Utils::ConvertToRGBA(perPixel(x, y));
			//m_imageData[x + y * m_finalImage->GetWidth()] = Utils::ConvertToRGBA(glm::vec3{0.1f, 0.2f, 0.9f});
		}
	}
#endif

	m_finalImage->SetData(m_imageData);
}


glm::vec3 Renderer::perPixel(uint32_t x, uint32_t y)
{

	Ray ray;
	Ray shadowRay;
	ray.origin = m_activeCamera->GetPosition();
	ray.direction = m_activeCamera->GetRayDirections()[x + y * m_finalImage->GetWidth()];
	//std::cout<<glm::length(ray.direction)<<std::endl;

	glm::vec3 color = glm::vec3{ 0.1f };//ambient light
	HitInfo hitInfo;
	int bounces = 3;
	for (int i = 0; i < bounces; i++) {
		//"reset" ray
		ray.t = -1;
		traceRay(ray, hitInfo);

		if (ray.t > 0) {
			ray.direction = glm::reflect(ray.direction, hitInfo.normal);
			//very important to avoid self-intersection by using an offset
			ray.origin = hitInfo.position + 0.0001f * ray.direction;
			
			
			for (const PointLight& pointLight : m_activeScene->lightSources) {
				shadowRay.direction = glm::normalize(pointLight.position - hitInfo.position);
				shadowRay.origin = hitInfo.position + 0.0001f * shadowRay.direction;
				if (!isInShadow(shadowRay))
					color += phongFull(hitInfo, m_activeCamera->GetPosition(), pointLight);
			}
		}
		else
			break;
		
	}


	return glm::clamp(color, glm::vec3{ 0.0f }, glm::vec3{ 1.0f });
}

//This function doesn't return anything, but changes ray.t and hitInfo
void Renderer::traceRay(Ray& ray, HitInfo& hitInfo)
{
	for (const auto& object : m_activeScene->objects)
	{
		if (std::holds_alternative<Sphere>(object)) {
			const Sphere sphere = std::get<Sphere>(object);
			intersectSphere(ray, sphere, hitInfo);
		}
		//TODO: add other objects
	}
	

}


bool Renderer::isInShadow(const Ray& ray) {

	for (const auto& object : m_activeScene->objects)
	{
		if (std::holds_alternative<Sphere>(object)) {
			const Sphere sphere = std::get<Sphere>(object);
			if (intersectSphere(ray, sphere))
				return true;
		}
		//TODO: add other objects
	}

	return false;
}

