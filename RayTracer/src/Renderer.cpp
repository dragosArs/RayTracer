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
	glm::vec3 color = glm::vec3{ 0.1f };//ambient light
	glm::vec3 reflectiveContribution = glm::vec3{ 1.0f };
	BasicHitInfo basicHitInfo;
	int bounces = 3;
	for (int i = 0; i < bounces; i++) {
		//"reset" ray
		ray.t = -1.0f;
		
		traceRay(ray, basicHitInfo);
		
		if (ray.t > 0.0f) {
			const FullHitInfo fullHitInfo = retrieveFullHitInfo(m_activeScene, basicHitInfo, ray);
			if (i >= 1) 
				reflectiveContribution = fullHitInfo.material.ks * reflectiveContribution;
			ray.direction = glm::reflect(ray.direction, fullHitInfo.normal);
			ray.origin =  fullHitInfo.position + 0.0000001f * ray.direction;

			for (const PointLight& pointLight : m_activeScene->lightSources) {
				shadowRay.direction = glm::normalize(pointLight.position - fullHitInfo.position);
				shadowRay.origin = fullHitInfo.position + 0.000001f * shadowRay.direction;
				//reflective component should contribute proportionally to the specular component of the material it reflects off of
				//if (!isInShadow(shadowRay))
					color += reflectiveContribution * phongFull(fullHitInfo, *m_activeCamera, pointLight);
			}
		}
		else
			i = bounces;
		
	}
	glm::vec3 finalColor = glm::clamp(color, glm::vec3{ 0.0f }, glm::vec3{ 1.0f });
	return finalColor;
}

//This function doesn't return anything, but changes ray.t and hitInfo
void Renderer::traceRay(Ray& ray, BasicHitInfo& hitInfo)
{
	for (const Sphere& sphere: m_activeScene->spheres)
	{
		intersectSphere(ray, sphere, hitInfo);
	}


	for (std::size_t i = 0; i < m_activeScene->triangles.size(); i++)
	{
		intersectTriangle(ray, hitInfo, *m_activeScene, i);
	}


}


bool Renderer::isInShadow(const Ray& ray) {

	for (const Sphere& sphere : m_activeScene->spheres)
	{
		if (intersectSphere(ray, sphere)) {
			return true;
		}
	}

	for (std::size_t i = 0; i < m_activeScene->triangles.size(); i++)
	{
		if(intersectTriangle(ray, *m_activeScene, i))
			return true;
	}

	return false;
}

FullHitInfo Renderer::retrieveFullHitInfo(const Scene* scene, const BasicHitInfo& basicHitInfo, const Ray& ray) {
	Triangle triangle = scene->triangles[basicHitInfo.triangleIndex];
	Vertex v0 = scene->vertices[triangle.vertexIndex0];
	Vertex v1 = scene->vertices[triangle.vertexIndex1];
	Vertex v2 = scene->vertices[triangle.vertexIndex2];
	float u = basicHitInfo.barU;
	float v = basicHitInfo.barV;

	return FullHitInfo{ ray.origin + ray.t * ray.direction,  u * v0.normal + v * v1.normal + (1 - u - v) * v2.normal, scene->materials[triangle.materialIndex] };
}

