#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Renderer.h"
#include "Shading.h"
#include "Draw.h"
#include "Calculation_utility.h"

#include "Walnut/Random.h"

#include <execution>
#include <iostream>
#include <queue>
#include <glm/gtc/type_ptr.hpp>

std::mutex coutMutex;

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

void Renderer::Render(const Scene& scene, const Camera& camera, const Settings& settings, const VisualDebugging& visDebugging)
{
	m_activeScene = &scene;
	m_activeCamera = &camera;
#define MT 0
#if DEBUG
	if (rayTraceMode)
	{
		for (uint32_t y = 0; y < m_finalImage->GetHeight(); y++)
		{
			for (uint32_t x = 0; x < m_finalImage->GetWidth(); x++)
			{
				m_imageData[x + y * m_finalImage->GetWidth()] = Utils::ConvertToRGBA(perPixel(x, y, false));
			}
		}
	}
	

#else 
	if (settings.enableRayTracing)
	{
		std::for_each(std::execution::par, m_imageVerticalIter.begin(), m_imageVerticalIter.end(),
			[this](uint32_t y)
			{
				std::for_each(std::execution::par, m_imageHorizontalIter.begin(), m_imageHorizontalIter.end(),
				[this, y](uint32_t x)
					{
						m_imageData[x + y * m_finalImage->GetWidth()] = Utils::ConvertToRGBA(perPixel(x, y, false));
					});
			});
	}
	else
	{
			std::for_each(std::execution::par, m_imageVerticalIter.begin(), m_imageVerticalIter.end(),
				[this](uint32_t y)
				{
					std::for_each(std::execution::par, m_imageHorizontalIter.begin(), m_imageHorizontalIter.end(),
					[this, y](uint32_t x)
						{
							m_imageData[x + y * m_finalImage->GetWidth()] = Utils::ConvertToRGBA(glm::vec3{0.0f});
						});
				});
	}
	std::unordered_map<Coord, float> zBuffer;
	if (visDebugging.enableWireframeTriangles)
	{		
		for (const std::pair<glm::vec3, glm::vec3>& line3D : DrawMeshes(scene))
		{
			std::pair<glm::vec3, glm::vec3> line2D = camera.ProjectLineOnScreen(line3D);
			if (line2D.first.z != -std::numeric_limits<double>::infinity() && line2D.second.z != - std::numeric_limits<double>::infinity()
				&& line2D.first.z != std::numeric_limits<double>::infinity() && line2D.second.z != std::numeric_limits<double>::infinity())
				Renderer::RasterizeLine(line2D.first, line2D.second, glm::vec3{ 1.0f, 0.0f, 0.0f }, zBuffer);
		}
	}
	if (visDebugging.enableWireframeBvh)
	{
		for (const std::pair<glm::vec3, glm::vec3>& line3D : DrawBvh(scene.bvh.get()))
		{
			std::pair<glm::vec3, glm::vec3> line2D = camera.ProjectLineOnScreen(line3D);
			if (line2D.first.z != -std::numeric_limits<double>::infinity() && line2D.second.z != -std::numeric_limits<double>::infinity()
				&& line2D.first.z != std::numeric_limits<double>::infinity() && line2D.second.z != std::numeric_limits<double>::infinity())
				Renderer::RasterizeLine(line2D.first, line2D.second, glm::vec3{ 0.0f, 0.0f, 1.0f }, zBuffer);
		}
	}
	

#endif

	

	m_finalImage->SetData(m_imageData);
}

void Renderer::Debug(const Scene& scene, const Camera& camera)
{
	m_activeScene = &scene;
	m_activeCamera = &camera;
	glm::vec3 finalColor = perPixel(camera.xDebug, camera.yDebug, true);
	std::cout << camera.xDebug << " " << camera.yDebug << std::endl;
	std::cout << "finalColor: " << finalColor.x << " " << finalColor.y << " " << finalColor.z << std::endl << std::endl;
}





void Renderer::RasterizeLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color, std::unordered_map<Coord, float>& zBuffer)
{
	//std::cout << "start: " << start.x << " " << start.y << std::endl;
	//std::cout << "end: " << end.x << " " << end.y << std::endl;
	int width = m_finalImage->GetWidth() - 1;
	int height = m_finalImage->GetHeight() - 1;
	Coord startClamped = Coord{ -1, -1 };
	Coord endClamped = Coord{ -1, -1};

	int dx = end.x - start.x;
	int dy = end.y - start.y;
	float p[4] = { -dx, dx, -dy, dy };
	float q[4] = { start.x, width - start.x, start.y, height - start.y };

	float t1 = 0.0f;
	float t2 = 1.0f;

	for (int i = 0; i < 4; i++)
	{
		if (p[i] == 0.0f && q[i] < 0.0f)
			return;
		if (p[i] < 0.0f)
		{
			float t = (q[i]) / (p[i]);  // This calculation was returning a zero because both q and p were int
			if (t > t1 && t < t2)
			{
				t1 = t;
			}
		}
		else if (p[i] > 0.0f)
		{
			float t = (q[i]) / (p[i]);  // This calculation was returning a zero because both q and p were int
			if (t > t1 && t < t2)
			{
				t2 = t;
			}
		}
	}

	if (t1 < t2) {
		startClamped = Coord{ (int)(start.x + t1 * dx), (int)(start.y + t1 * dy) };
		endClamped = Coord{ (int)(start.x + t2 * dx), (int)(start.y + t2 * dy) };
	}
	else
		return;

	//std::cout << "startClamped: " << startClamped.x << " " << startClamped.y << std::endl;
	//std::cout << "endClamped: " << endClamped.x << " " << endClamped.y << std::endl << std::endl;
	dx = abs(endClamped.x - startClamped.x);
	dy = abs(endClamped.y - startClamped.y);
	int sx = (startClamped.x < endClamped.x) ? 1 : -1;
	int sy = (startClamped.y < endClamped.y) ? 1 : -1;
	int error = dx - dy;
	float dist = glm::distance(glm::vec2(start.x, start.y), glm::vec2(end.x, end.y));
	int x = startClamped.x;
	int y = startClamped.y;

	while (x != endClamped.x || y != endClamped.y) {
		// Process the current point (x1, y1)
		float t = glm::distance(glm::vec2(x, y), glm::vec2(startClamped.x, startClamped.y)) / dist;
		float depth = t * start.z + (1 - t) * end.z;
		if (x > 0 && x < width && y > 0 && y < height && (zBuffer.count(Coord{ x, y }) == 0 || depth < zBuffer[Coord{x, y}]))
		{
			zBuffer[Coord{ x, y }] = depth;
			m_imageData[x + y * m_finalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}

		int error2 = 2 * error;

		// If the error is greater than zero, it means it's time
		// to move vertically.
		if (error2 > -dy) {
			error -= dy;
			x += sx;
		}

		// If the error is less than dx, it's time to move horizontally.
		if (error2 < dx) {
			error += dx;
			y += sy;
		}
	}

	// Process the final point (x2, y2)
	float t = glm::distance(glm::vec2(x, y), glm::vec2(start.x, start.y)) / dist;
	float depth = t * start.z + (1 - t) * end.z;
	if (x > 0 && x < width && y > 0 && y < height && (zBuffer.count(Coord{ x, y }) == 0 || depth < zBuffer[Coord{ x, y }]))
	{
		zBuffer[Coord{ x, y }] = depth;
		m_imageData[x + y * m_finalImage->GetWidth()] = Utils::ConvertToRGBA(color);
	}
	
	//std::cout << start.x << " " << start.y << " " << end.x << " " << end.y << std::endl;
}


glm::vec3 Renderer::perPixel(uint32_t x, uint32_t y, bool debug)
{

	Ray ray;
	Ray shadowRay;
	ray.origin = m_activeCamera->GetPosition();
	ray.direction = m_activeCamera->GetRayDirections()[x + y * m_finalImage->GetWidth()];
	ray.invDirection = glm::vec3{ 1.0f } / ray.direction;
	glm::vec3 color = glm::vec3{ 0.1f };//ambient light
	glm::vec3 reflectiveContribution = glm::vec3{ 1.0f };
	BasicHitInfo basicHitInfo;
	int bounces = 3;
	for (int i = 0; i < bounces; i++)
	{
		//"reset" ray
		ray.t = -1.0f;
		traceRay(ray, basicHitInfo, debug);
		if(debug)
			std::cout << "ray.t: " << ray.t << std::endl;
		FullHitInfo fullHitInfo;
		if (ray.t > EPSILON )
		{
			fullHitInfo = retrieveFullHitInfo(m_activeScene, basicHitInfo, ray);
			ray.direction = glm::reflect(ray.direction, fullHitInfo.normal);
			ray.origin =  fullHitInfo.position + EPSILON * ray.direction;
			ray.invDirection = glm::vec3{ 1.0f } / ray.direction;

			for (const PointLight& pointLight : m_activeScene->lightSources)
			{
				shadowRay.origin = fullHitInfo.position + 0.075f * fullHitInfo.normal + 100 * EPSILON * shadowRay.direction;
				float length = glm::length(pointLight.position - shadowRay.origin);
				shadowRay.direction = glm::normalize(pointLight.position - shadowRay.origin);
				shadowRay.invDirection = glm::normalize(glm::vec3{ 1.0f } / shadowRay.direction);
				//If Epsilon is small teapot is rendered well, but cornell box is not
				//If Epsilon is big teapot is not rendered well, but cornell box is
				//Wtf
				//How do I solve self intersection problem?
				//Maybe keep track of the last hit object and if it's the same as the current one, ignore it?
				if (!isInShadow(shadowRay, length, debug, basicHitInfo.triangleIndex))
					color += reflectiveContribution * phongFull(fullHitInfo, *m_activeCamera, pointLight);
			}
			
			if (fullHitInfo.material.ks != glm::vec3{0.0f})
				reflectiveContribution = fullHitInfo.material.ks * reflectiveContribution;
			else
				break;
		}
		else
			i = bounces;
	}
	glm::vec3 finalColor = glm::clamp(color, glm::vec3{ 0.0f }, glm::vec3{ 1.0f });
	return finalColor;
}


//This function doesn't return anything, but changes ray.t and hitInfo
void Renderer::traceRay(Ray& ray, BasicHitInfo& hitInfo, bool debug)
{
	std::queue<const BVH*> queue;
	queue.push(m_activeScene->bvh.get());

	while (queue.size() > 0) 
	{
		const BVH* cur = queue.front();
		queue.pop();
		const BVH* leftBvh = cur->left.get();
		const BVH* rightBvh = cur->right.get();

		if (leftBvh->triangleIndex == -1)
		{
			if (intersectAABB(ray, leftBvh->boundingBox, debug))
				queue.push(leftBvh);
		}
		else {
			intersectTriangle(ray, hitInfo, *m_activeScene, leftBvh->triangleIndex, debug);
			if(debug)
				std::cout << ray.t << std::endl;
		}
		

		if (rightBvh->triangleIndex == -1)
		{
			if (intersectAABB(ray, rightBvh->boundingBox, debug))
				queue.push(rightBvh);
		}
		else {
			intersectTriangle(ray, hitInfo, *m_activeScene, rightBvh->triangleIndex, debug);
			if(debug)
				std::cout << ray.t << std::endl;
		}
	}
}


bool Renderer::isInShadow(const Ray& ray, float length, bool debug, uint32_t originalTriangleIndex) 
{
	std::queue<const BVH*> queue;
	queue.push(m_activeScene->bvh.get());

	while (queue.size() > 0) {
		const BVH* cur = queue.front();
		queue.pop();
		const BVH* leftBvh = cur->left.get();
		const BVH* rightBvh = cur->right.get();

		if (leftBvh->triangleIndex == -1)
		{
			if (intersectAABB(ray, leftBvh->boundingBox, debug))
				queue.push(leftBvh);
		}
		else if (leftBvh->triangleIndex != originalTriangleIndex && intersectTriangle(ray, *m_activeScene, leftBvh->triangleIndex, length, debug))
		{
			//std::lock_guard<std::mutex> lock(coutMutex);
			//std::cout << "Current triangle index: " << leftBvh->triangleIndex << ", Original triangle index: " << originalTriangleIndex << std::endl;
			return true;
		}

		if (rightBvh->triangleIndex == -1)
		{
			if (intersectAABB(ray, rightBvh->boundingBox, debug))
				queue.push(rightBvh);
		}
		else if (rightBvh->triangleIndex != originalTriangleIndex && intersectTriangle(ray, *m_activeScene, rightBvh->triangleIndex, length, debug))
		{
			//std::lock_guard<std::mutex> lock(coutMutex);
			//std::cout << "Current triangle index: " << rightBvh->triangleIndex << ", Original triangle index: " << originalTriangleIndex << std::endl;
			return true;
		}
	}

	return false;
}

//TODO
//Apply bilinear interpolation here
FullHitInfo Renderer::retrieveFullHitInfo(const Scene* scene, const BasicHitInfo& basicHitInfo, const Ray& ray) {
	Triangle triangle = scene->triangles[basicHitInfo.triangleIndex];
	Vertex v0 = scene->vertices[triangle.vertexIndex0];
	Vertex v1 = scene->vertices[triangle.vertexIndex1];
	Vertex v2 = scene->vertices[triangle.vertexIndex2];
	float u = basicHitInfo.barU;
	float v = basicHitInfo.barV;
	glm::vec3 hitPos = ray.origin + ray.t * ray.direction;
	glm::vec3 normal = glm::normalize((1 - u - v) * v0.normal + u * v1.normal +  v * v2.normal);
	return FullHitInfo{ hitPos, normal, scene->materials[triangle.materialIndex] };
}



