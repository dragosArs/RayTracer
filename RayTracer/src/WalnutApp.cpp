#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h"

#include <glm/gtc/type_ptr.hpp>
#include "rapidobj.hpp"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f)
	{

		loadScene("\\assets\\objects\\CornellBox-Mirror-Rotated.obj", "\\assets\\materials\\CornellBox-Mirror-Rotated.mtl", m_Scene);
		//loadScene("\\assets\\objects\\teapot.obj", "\\assets\\materials\\default.mtl", m_Scene);
		
		{
			PointLight pointLight;
			pointLight.position = { 0.0f, 35.0f, 0.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}

		{
			PointLight pointLight;
			pointLight.position = { 2.0f, 5.0f, 20.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}

		{
			PointLight pointLight;
			pointLight.position = { 0.0f, 25.0f, -21.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}

		{
			PointLight pointLight;
			pointLight.position = { -22.0f, -23.0f, -1.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}

		{
			PointLight pointLight;
			pointLight.position = { -2.0f, -23.0f, 3.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}
		
		{
			PointLight pointLight;
			pointLight.position = { 0.005f, 1.98f, 0.0325f };
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
		if (ImGui::Button("Debug"))
		{
			Debug();
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

	void Debug()
	{
		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Debug(m_Scene, m_Camera);
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

