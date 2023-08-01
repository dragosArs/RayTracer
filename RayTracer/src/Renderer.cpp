#include "Renderer.h"

#include "Walnut/Random.h"

#include <execution>

namespace Utils {

	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}

}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// No resize necessary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	m_ImageHorizontalIter.resize(width);
	m_ImageVerticalIter.resize(height);
	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIter[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIter[i] = i;
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

#define MT 1
#if MT
	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this](uint32_t y)
		{
			std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
			[this, y](uint32_t x)
				{
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(PerPixel(x, y));
				});
		});

#else

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(PerPixel(x, y));
		}
	}
#endif

	m_FinalImage->SetData(m_ImageData);
}

//TODO implement this function to return the color of the pixel at (x, y)
glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.origin = m_ActiveCamera->GetPosition();
	ray.direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	return glm::vec4(1.0f);
}

//TODO
//Iterate through all objects in the scene and find the closest hit
Renderer::HitInfo Renderer::TraceRay(const Ray& ray)
{
	Renderer::HitInfo hitInfo;
	return hitInfo;
}

//TODO
//Return information about the point where a ray intersects an object
Renderer::HitInfo Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	Renderer::HitInfo hitInfo;
	return hitInfo;
}
