#include "Camera.h"
#include "Ray.h"

#include <glm/gtx/quaternion.hpp>

#include "Walnut/Input/Input.h"
#include "Calculation_utility.h"

using namespace Walnut;

Camera::Camera(float verticalFOV, float nearClip, float farClip)
	: m_VerticalFOV(verticalFOV), m_NearClip(nearClip), m_FarClip(farClip)
{
	//IMPORTANT: Tweak camera variables here
	m_ForwardDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	m_Position = glm::vec3(0.0f, 0.0f, 3.0f);
	xDebug = 0;
	yDebug = 0;
}

void Camera::OnUpdate(float ts)
{
	glm::vec2 mousePos = Input::GetMousePosition();
	glm::vec2 delta = (mousePos - m_LastMousePosition) * 0.002f;
	//debug specific points

	m_LastMousePosition = mousePos;

	if (!Input::IsMouseButtonDown(MouseButton::Right))
	{
		Input::SetCursorMode(CursorMode::Normal);
		return;
	}

	Input::SetCursorMode(CursorMode::Locked);

	bool moved = false;

	constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
	glm::vec3 rightDirection = glm::cross(m_ForwardDirection, upDirection);

	float speed = 5.0f;

	// Movement
	if (Input::IsKeyDown(KeyCode::W))
	{
		m_Position += m_ForwardDirection * speed * ts;
		moved = true;
	}
	else if (Input::IsKeyDown(KeyCode::S))
	{
		m_Position -= m_ForwardDirection * speed * ts;
		moved = true;
	}
	if (Input::IsKeyDown(KeyCode::A))
	{
		m_Position -= rightDirection * speed * ts;
		moved = true;
	}
	else if (Input::IsKeyDown(KeyCode::D))
	{
		m_Position += rightDirection * speed * ts;
		moved = true;
	}
	if (Input::IsKeyDown(KeyCode::Q))
	{
		m_Position -= upDirection * speed * ts;
		moved = true;
	}
	else if (Input::IsKeyDown(KeyCode::E))
	{
		m_Position += upDirection * speed * ts;
		moved = true;
	}

	//used for debugging specific pixel that mouse points at
	if (Input::IsKeyDown(KeyCode::R))
	{
		//get the position of the mouse in world space
		xDebug = (uint32_t) Input::GetMousePosition().x;
		yDebug = (uint32_t) Input::GetMousePosition().y;
	}

	// Rotation
	if (delta.x != 0.0f || delta.y != 0.0f)
	{
		float pitchDelta = delta.y * GetRotationSpeed();
		float yawDelta = delta.x * GetRotationSpeed();

		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
			glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
		m_ForwardDirection = glm::rotate(q, m_ForwardDirection);

		moved = true;
	}

	if (moved)
	{
		RecalculateView();
		RecalculateRayDirections();
	}
}

void Camera::OnResize(uint32_t width, uint32_t height)
{
	if (width == m_ViewportWidth && height == m_ViewportHeight)
		return;

	m_ViewportWidth = width;
	m_ViewportHeight = height;

	RecalculateProjection();
	RecalculateRayDirections();
}

float Camera::GetRotationSpeed()
{
	return 0.3f;
}

void Camera::RecalculateProjection()
{
	m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::RecalculateView()
{
	m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
	m_InverseView = glm::inverse(m_View);
}


void Camera::RecalculateRayDirections()
{
	m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);

	for (uint32_t y = 0; y < m_ViewportHeight; y++)
	{
		for (uint32_t x = 0; x < m_ViewportWidth; x++)
		{
			glm::vec2 coord = { (float)x / (float)m_ViewportWidth, (float)y / (float)m_ViewportHeight };
			coord = coord * 2.0f - 1.0f; // -1 -> 1

			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space
			m_RayDirections[x + y * m_ViewportWidth] = rayDirection;
		}
	}
}

//void Camera::sampleFocusJitter(int samples)
//{
//	m_FocusPositions.clear();
//	//TODO CHANGE THIS
//	constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
//	glm::vec3 jitter = glm::cross(m_ForwardDirection, upDirection);
//	glm::quat rotation = glm::angleAxis(glm::radians(360.0f / samples), m_ForwardDirection);
//
//	for (int i = 0; i < samples; i++)
//	{
//		jitter = rotation * jitter;
//		m_FocusPositions.push_back(m_Position + aperture * jitter);
//	}
//}

void Camera::sampleFocusJitter(int samples)
{
	m_FocusPositions.clear();
	std::vector<glm::vec3> jitterSamples = createJitter(12345);
	glm::quat rot = glm::rotation(glm::vec3{0, 1, 0}, m_ForwardDirection);
	for (int i = 0; i < samples; i++)
	{
		glm::vec3 translatedVec = rot * jitterSamples[i];
		m_FocusPositions.emplace_back(glm::vec3{m_Position + aperture * translatedVec});
	}
}

const std::pair<glm::vec3, glm::vec3> Camera::ProjectLineOnScreen(std::pair<glm::vec3, glm::vec3> line) const
{
	glm::vec4 viewport = glm::vec4(0, 0, m_ViewportWidth, m_ViewportHeight);
	return std::make_pair(
		glm::project(line.first, GetView(), GetProjection(), viewport),
		glm::project(line.second, GetView(), GetProjection(), viewport)
	);
};