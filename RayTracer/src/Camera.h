#pragma once

#include <glm/glm.hpp>
#include <vector>

class Camera
{
public:
	Camera(float verticalFOV, float nearClip, float farClip);

	void OnUpdate(float ts);
	void OnResize(uint32_t width, uint32_t height);

	const glm::mat4& GetProjection() const { return m_Projection; }
	const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
	const glm::mat4& GetView() const { return m_View; }
	const glm::mat4& GetInverseView() const { return m_InverseView; }
	const glm::vec3& GetPosition() const { return m_Position; }
	const glm::vec3& GetDirection() const { return m_ForwardDirection; }
	const float GetFarClip() const { return m_FarClip; }
	const float GetNearClip() const { return m_NearClip; }
	const std::vector<glm::vec3>& GetFocusPositions() const { return m_FocusPositions; }
	void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateView(); RecalculateRayDirections(); }
	void SetDirection(const glm::vec3& direction) { m_ForwardDirection = direction; RecalculateView(); RecalculateRayDirections(); }
	const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; }
	const std::pair<glm::vec3, glm::vec3> ProjectLineOnScreen(std::pair<glm::vec3, glm::vec3> line) const;
	void sampleFocusJitter(int samples);

	 
	uint32_t xDebug = 0;
	uint32_t yDebug = 0;
	float aperture = 0.1f;
	float focalLength = 10.0f;
	float GetRotationSpeed();
private:
	void RecalculateProjection();
	void RecalculateView();
	void RecalculateRayDirections();
private:
	glm::mat4 m_Projection{ 1.0f };
	glm::mat4 m_View{ 1.0f };
	glm::mat4 m_InverseProjection{ 1.0f };
	glm::mat4 m_InverseView{ 1.0f };

	float m_VerticalFOV = 45.0f;
	float m_NearClip = 0.1f;
	float m_FarClip = 100.0f;


	glm::vec3 m_Position{0.0f, 0.0f, 3.0f};
	glm::vec3 m_ForwardDirection{0.0f, 0.0f, -1.0f};

	// Cached ray directions
	std::vector<glm::vec3> m_RayDirections;
	std::vector<glm::vec3> m_FocusPositions;
	glm::vec2 m_LastMousePosition{ 0.0f, 0.0f };

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
};