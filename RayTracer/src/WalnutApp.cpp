#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f)
	{
		Material pinkSphere;
		pinkSphere.Albedo = { 1.0f, 0.0f, 1.0f };
		pinkSphere.Roughness = 0.3f;
		pinkSphere.Metallic = 0.2f;

		Material blueSphere;
		blueSphere.Albedo = { 0.2f, 0.0f, 1.0f };
		blueSphere.Roughness = 0.001f;
		blueSphere.Metallic = 0.3f;

		Material greenSphere;
		greenSphere.Albedo = { 0.2f, 1.0f, 0.0f };
		greenSphere.Roughness = 0.5f;
		greenSphere.Metallic = 0.1f;

		{
			Sphere sphere;
			sphere.center = { 0.0f, 1.0f, 0.0f };
			sphere.radius = 1.0f;
			sphere.material = blueSphere;
			m_Scene.objects.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.center = { 5.0f, 1.0f, -3.0f };
			sphere.radius = 1.0f;
			sphere.material = greenSphere;
			m_Scene.objects.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.center = { 0.0f, -101.0f, 0.0f };
			sphere.radius = 100.0f;
			sphere.material = pinkSphere;
			m_Scene.objects.push_back(sphere);
		}

		{
			PointLight pointLight;
			pointLight.position = { 0.0f, 3.0f, 0.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}
	}

	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render"))
		{
			Render();
		}
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();
		if (image)
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() },
				ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}
private:
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
				{
					app->Close();
				}
				ImGui::EndMenu();
			}
		});
	return app;
}

