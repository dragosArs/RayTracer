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
#include <stack>

std::mutex cMutex;
namespace Utils
{

	static uint32_t ConvertToRGBA(const glm::vec3& color)
	{
		uint8_t r = static_cast<uint8_t>(color.r * 255.0f);
		uint8_t g = static_cast<uint8_t>(color.g * 255.0f);
		uint8_t b = static_cast<uint8_t>(color.b * 255.0f);
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
#if MT
	if (m_settings.enableRayTracing)
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
	if (m_settings.enableRayTracing)
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

		if (m_visualDebugging.enableRaysDebugging)
		{
			std::unordered_map<Coord, float> zBuffer;
			for (const std::tuple<glm::vec3, glm::vec3, glm::vec3>& line3D :m_visualDebugging.debugRays)
			{
				std::pair<glm::vec3, glm::vec3> line2D = camera.ProjectLineOnScreen(std::pair<glm::vec3, glm::vec3>{std::get<0>(line3D), std::get<1>(line3D)});
				glm::vec3 color = std::get<2>(line3D);
				if (line2D.first.z != -std::numeric_limits<double>::infinity() && line2D.second.z != -std::numeric_limits<double>::infinity()
					&& line2D.first.z != std::numeric_limits<double>::infinity() && line2D.second.z != std::numeric_limits<double>::infinity())
					Renderer::RasterizeLine(line2D.first, line2D.second, color, zBuffer);
			}
		}
	}
	else
	{
		std::for_each(std::execution::par, m_imageVerticalIter.begin(), m_imageVerticalIter.end(),
			[this](uint32_t y)
			{
				std::for_each(std::execution::par, m_imageHorizontalIter.begin(), m_imageHorizontalIter.end(),
				[this, y](uint32_t x)
					{
						m_imageData[x + y * m_finalImage->GetWidth()] = Utils::ConvertToRGBA(glm::vec3{ 0.0f });
					});
			});
	}
	std::unordered_map<Coord, float> zBuffer;
	if (m_visualDebugging.enableWireframeTriangles)
	{		
		for (const std::pair<glm::vec3, glm::vec3>& line3D : DrawMeshes(scene))
		{
			std::pair<glm::vec3, glm::vec3> line2D = camera.ProjectLineOnScreen(line3D);
			if (line2D.first.z != -std::numeric_limits<double>::infinity() && line2D.second.z != - std::numeric_limits<double>::infinity()
				&& line2D.first.z != std::numeric_limits<double>::infinity() && line2D.second.z != std::numeric_limits<double>::infinity())
				Renderer::RasterizeLine(line2D.first, line2D.second, glm::vec3{ 0.0f, 1.0f, 0.0f }, zBuffer);
		}
	}
	if (m_visualDebugging.enableWireframeBvh)
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
	m_visualDebugging.debugRays = debugPixel(camera.xDebug, camera.yDebug);
	std::cout << camera.xDebug << " " << camera.yDebug << "\n";
}


glm::vec3 Renderer::perPixel(uint32_t x, uint32_t y, bool debug)
{

	Ray ray;
	Ray shadowRay;
	ray.origin = m_activeCamera->GetPosition();
	ray.direction = m_activeCamera->GetRayDirections()[x + y * m_finalImage->GetWidth()];
	ray.invDirection = glm::vec3{ 1.0f } / ray.direction;
	glm::vec3 color = glm::vec3{ 0.0f };//ambient light
	glm::vec3 reflectiveContribution = glm::vec3{ 1.0f };
	BasicHitInfo basicHitInfo;
	for (int i = 0; i <= m_settings.bounces; i++)
	{
		//"reset" ray
		ray.t = 1000.f;
		traceRay(ray, basicHitInfo, debug);
		FullHitInfo fullHitInfo;
		if (ray.t > EPSILON && ray.t < 1000.f)
		{
			fullHitInfo = retrieveFullHitInfo(m_activeScene, basicHitInfo, ray);
			ray.direction = glm::reflect(ray.direction, fullHitInfo.normal);
			ray.origin =  fullHitInfo.position + EPSILON * ray.direction;
			ray.invDirection = glm::normalize(glm::vec3{ 1.0f } / ray.direction);

			shadowRay.origin = fullHitInfo.position + EPSILON * fullHitInfo.normal;
			for (const PointLight& pointLight : m_activeScene->pointLightSources)
			{
				
				float length = glm::length(pointLight.position - shadowRay.origin);
				shadowRay.direction = (pointLight.position - shadowRay.origin) / length;
				shadowRay.invDirection = glm::normalize(glm::vec3{ 1.0f } / shadowRay.direction);

				if (!m_settings.enableShadows || !isInShadow(shadowRay, length, debug, basicHitInfo.triangleIndex))
					color += reflectiveContribution * phongFull(fullHitInfo, *m_activeCamera, pointLight);
			}

			for (const ParallelogramLight& pl : m_activeScene->parallelogramLightSources)
			{
				glm::vec3 accumulatedColor = glm::vec3{ 0.0f };
				int detail = m_settings.lightSamples;
				int numberOfSamples = detail * detail;
				PointLight samplePointLight{};
				for (int i = 0; i < numberOfSamples; ++i)
				{
					int x = i % detail;
					int y = i / detail;
					float u = (x + 0.5f) / detail;
					float v = (y + 0.5f) / detail;
					//cMutex.lock();
					//std::cout << u << " " << v << "\n";
					samplePointLight.position = pl.v0 + u * pl.edge1 + v * pl.edge2;
					samplePointLight.color = ((1 - u) * (1 - v) * pl.color0 + (1 - u) * v * pl.color1 + u * (1 - v) * pl.color2 + u * v * pl.color3);
					float length = glm::length(samplePointLight.position - shadowRay.origin);
					shadowRay.direction = (samplePointLight.position - shadowRay.origin) / length;
					shadowRay.invDirection = glm::normalize(glm::vec3{ 1.0f } / shadowRay.direction);

					if (!m_settings.enableShadows || !isInShadow(shadowRay, length, debug, basicHitInfo.triangleIndex))
						accumulatedColor += phongFull(fullHitInfo, *m_activeCamera, samplePointLight);
				}
				
				color += reflectiveContribution * accumulatedColor / (float)numberOfSamples;
			}
			
			if (fullHitInfo.material.ks != glm::vec3{0.0f})
				reflectiveContribution *=  reflectiveContribution;
			else
				break;
		}  
		else
			i = m_settings.bounces;
	}
	glm::vec3 finalColor = glm::clamp(color, glm::vec3{ 0.0f }, glm::vec3{ 1.0f });
	return finalColor;
}

void Renderer::traceRay(Ray& ray, BasicHitInfo& hitInfo, bool debug)
{
	std::stack<const BVH*> stack;
	stack.push(m_activeScene->bvh.get());

	while (stack.size() > 0)
	{
		const BVH* cur = stack.top();
		stack.pop();
		const BVH* leftBvh = cur->left.get();
		const BVH* rightBvh = cur->right.get();

		if (leftBvh->triangleIndex == -1)
		{
			float dist = intersectAABB(ray, leftBvh->boundingBox);
			if (dist > 0 && dist < ray.t)
				stack.push(leftBvh);
		}
		else {
			intersectTriangle(ray, hitInfo, *m_activeScene, leftBvh->triangleIndex);
		}


		if (rightBvh->triangleIndex == -1)
		{
			float dist = intersectAABB(ray, rightBvh->boundingBox);
			if (dist > 0 && dist < ray.t)
				stack.push(rightBvh);
		}
		else {
			intersectTriangle(ray, hitInfo, *m_activeScene, rightBvh->triangleIndex);
		}
	}
}


bool Renderer::isInShadow(const Ray& ray, float length, bool debug, uint32_t originalTriangleIndex) 
{
	std::stack<const BVH*> stack;
	stack.push(m_activeScene->bvh.get());

	while (stack.size() > 0) {
		const BVH* cur = stack.top();
		stack.pop();
		const BVH* leftBvh = cur->left.get();
		const BVH* rightBvh = cur->right.get();

		if (leftBvh->triangleIndex == -1)
		{
			if (intersectAABB(ray, leftBvh->boundingBox) > 0)
				stack.push(leftBvh);
		}
		else if (leftBvh->triangleIndex != originalTriangleIndex && intersectTriangle(ray, *m_activeScene, leftBvh->triangleIndex, length))
		{
			return true;
		}

		if (rightBvh->triangleIndex == -1)
		{
			if (intersectAABB(ray, rightBvh->boundingBox) > 0)
				stack.push(rightBvh);
		}
		else if (rightBvh->triangleIndex != originalTriangleIndex && intersectTriangle(ray, *m_activeScene, rightBvh->triangleIndex, length))
		{
			return true;
		}
	}

	return false;
}

FullHitInfo Renderer::retrieveFullHitInfo(const Scene* scene, const BasicHitInfo& basicHitInfo, const Ray& ray)
{
	const Triangle& triangle = scene->triangles[basicHitInfo.triangleIndex];
	const Vertex& v0 = scene->vertices[triangle.vertexIndex0];
	const Vertex& v1 = scene->vertices[triangle.vertexIndex1];
	const Vertex& v2 = scene->vertices[triangle.vertexIndex2];
	float u = basicHitInfo.barU;
	float v = basicHitInfo.barV;
	const glm::vec3 hitPos = ray.origin + ray.t * ray.direction;
	glm::vec3 normal;
	if(v0.normal == glm::vec3{0.0f} || v1.normal == glm::vec3{0.0f} || v2.normal == glm::vec3{0.0f})
		normal = glm::normalize(glm::cross(v1.position - v0.position, v2.position - v0.position));
	else
		normal = glm::normalize((1 - u - v) * v0.normal + u * v1.normal +  v * v2.normal);
	Material mat = scene->materials[triangle.materialIndex];
	if (m_settings.applyTexture)
	{
		glm::vec2 pixelCoord = (1 - u - v) * v0.texCoord + u * v1.texCoord + v * v2.texCoord;
		if(mat.diffuseMapIndex >= 0)
			mat.kd = applyBilinearInterpolation(pixelCoord, scene->diffuseMaps[mat.diffuseMapIndex]);
		/*if (mat.normalMapIndex >= 0)
			normal = applyBilinearInterpolation(pixelCoord, scene->normalMaps[mat.normalMapIndex]);*/
	}
	
		
	return FullHitInfo{ hitPos, normal, mat };
}

glm::vec3 Renderer::applyBilinearInterpolation(const glm::vec2& pixelCoord, const Texture& texture)
{

	float texelX = fmod(pixelCoord.x * (texture.width - 1), texture.width);
	float texelY = fmod(pixelCoord.y * (texture.height - 1), texture.height);
	if(texelX < 0)
		texelX += texture.width;
	if (texelY < 0)
		texelY += texture.height;

	// Convert to image coordinates
	int x0 = static_cast<int>(texelX);
	int y0 = static_cast<int>(texelY);
	int x1 = x0 < texture.width - 1 ? x0 + 1 : x0;
	int y1 = y0 < texture.height - 1 ? y0 + 1 : y0;

	float u = texelX - x0;
	float v = texelY - y0;
	
	int tl = y0 * texture.width + x0;
	int tr = y0 * texture.width + x1;
	int bl = y1 * texture.width + x0;
	int br = y1 * texture.width + x1;

	glm::vec3 colorTl = texture.pixels[tl];
	glm::vec3 colorTr = texture.pixels[tr];
	glm::vec3 colorBl = texture.pixels[bl];
	glm::vec3 colorBr = texture.pixels[br];

	glm::vec3 color = (1 - u) * (1 - v) * colorTl + u * (1 - v) * colorTr + (1 - u) * v * colorBl + u * v * colorBr;
	return color;
}

std::vector<std::tuple<glm::vec3, glm::vec3, glm::vec3>> Renderer::debugPixel(uint32_t x, uint32_t y)
{
	std::vector<std::tuple<glm::vec3, glm::vec3, glm::vec3>> debugLines;

	Ray ray;
	Ray shadowRay;
	ray.origin = m_activeCamera->GetPosition();
	ray.direction = m_activeCamera->GetRayDirections()[x + y * m_finalImage->GetWidth()];
	BasicHitInfo basicHitInfo;
	glm::vec3 color;
	int bounces = 3;
	for (int i = 0; i < bounces; i++)
	{
		//"reset" ray
		ray.t = -1.0f;
		traceRay(ray, basicHitInfo, true);
		color = glm::vec3{ 0.0f };
		FullHitInfo fullHitInfo;
		glm::vec3 rayOrigDebug = ray.origin;
		glm::vec3 rayDirDebug = glm::normalize(ray.direction);
		glm::vec3 color = glm::vec3{ 1.0f, 0.0f, 0.0f };
		if (ray.t > EPSILON)
		{
			fullHitInfo = retrieveFullHitInfo(m_activeScene, basicHitInfo, ray);
			ray.direction = glm::reflect(ray.direction, fullHitInfo.normal);
			ray.origin = fullHitInfo.position + EPSILON * ray.direction;
			ray.invDirection = glm::vec3{ 1.0f } / ray.direction;

			for (const PointLight& pointLight : m_activeScene->pointLightSources)
			{
				shadowRay.origin = fullHitInfo.position + 0.075f * fullHitInfo.normal + 100 * EPSILON * shadowRay.direction;
				float length = glm::length(pointLight.position - shadowRay.origin);
				shadowRay.direction = glm::normalize(pointLight.position - shadowRay.origin);
				shadowRay.invDirection = glm::normalize(glm::vec3{ 1.0f } / shadowRay.direction);

				if (!isInShadow(shadowRay, length, true, basicHitInfo.triangleIndex))
				{
					color = phongFull(fullHitInfo, *m_activeCamera, pointLight);
					debugLines.push_back(std::tuple<glm::vec3, glm::vec3, glm::vec3>{fullHitInfo.position, pointLight.position, glm::vec3{ 1.0f, 1.0f, 1.0f }});
				}
				else
					debugLines.push_back(std::tuple<glm::vec3, glm::vec3, glm::vec3>{fullHitInfo.position, pointLight.position, glm::vec3{ 0.0f, 1.0f, 1.0f }});
			}
		}
		else
			i = bounces;
		debugLines.push_back(std::tuple<glm::vec3, glm::vec3, glm::vec3>{rayOrigDebug, rayOrigDebug + ray.t * rayDirDebug, color});
	}

	return debugLines;
}


void Renderer::RasterizeLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color, std::unordered_map<Coord, float>& zBuffer)
{
	//std::cout << "start: " << start.x << " " << start.y << "\n";
	//std::cout << "end: " << end.x << " " << end.y << "\n";
	int width = m_finalImage->GetWidth() - 1;
	int height = m_finalImage->GetHeight() - 1;
	Coord startClamped = Coord{ -1, -1 };
	Coord endClamped = Coord{ -1, -1 };

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

	//std::cout << "startClamped: " << startClamped.x << " " << startClamped.y << "\n";
	//std::cout << "endClamped: " << endClamped.x << " " << endClamped.y << "\n" << "\n";
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
		if (x > 0 && x < width && y > 0 && y < height && (zBuffer.count(Coord{ x, y }) == 0 || depth < zBuffer[Coord{ x, y }]))
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

	//std::cout << start.x << " " << start.y << " " << end.x << " " << end.y << "\n";
}


