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
		
#define SCENE 1
#if SCENE
		loadScene("\\assets\\objects\\teapot.obj", "\\assets\\materials\\default.mtl", m_Scene);
		
		/*m_Camera.SetDirection({ 0.0f, 0.0f, -1.0f });
		m_Camera.SetPosition({ 0.0f, 0.0f, 30.0f });*/

		m_Camera.SetDirection({ 0.0f, 0.0f, -1.0f });
		m_Camera.SetPosition({ 0.0f, 0.0f, 50.0f });
		/*
		{
			PointLight pointLight;
			pointLight.position = { 0.0f, 35.0f, 0.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}

		{
			PointLight pointLight;
			pointLight.position = { -3.0f, 5.0f, 20.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}

		{
			PointLight pointLight;
			pointLight.position = { 0.0f, -20.0f, 0.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}
		*/
		{
			PointLight pointLight;
			pointLight.position = { 0.0f, 0.0f, 30.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}

		/*{
			PointLight pointLight;
			pointLight.position = { 0.0f, 0.0f, -30.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}

		{
			PointLight pointLight;
			pointLight.position = { 0.0f, 30.0f, 0.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}*/

		/*{
			PointLight pointLight;
			pointLight.position = { 2.0f, 3.0f, -2.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}

		{
			PointLight pointLight;
			pointLight.position = { -2.0f, 3.0f, 2.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}

		{
			PointLight pointLight;
			pointLight.position = { -2.0f, 3.0f, -2.0f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}*/
#else
		m_Camera.SetPosition({ 0.0f, 0.0f, -3.0f });
		m_Camera.SetDirection({ 0.0f, 0.0f, 1.0f });
		loadScene("\\assets\\objects\\CornellBox-Mirror-Rotated.obj", "\\assets\\materials\\CornellBox-Mirror-Rotated.mtl", m_Scene);
		{
			PointLight pointLight;
			pointLight.position = { 0.005f, 1.98f, 0.0325f };
			pointLight.color = { 1.0f, 1.0f, 1.0f };
			m_Scene.lightSources.push_back(pointLight);
		}
#endif
		
		
		
	}

	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		ImGui::Text("Camera Position: %.3f, %.3f, %.3f", m_Camera.GetPosition().x, m_Camera.GetPosition().y, m_Camera.GetPosition().z);
		ImGui::Text("Camera Direction: %.3f, %.3f, %.3f", m_Camera.GetDirection().x, m_Camera.GetDirection().y, m_Camera.GetDirection().z);
		
		ImGui::Text("Visual debugging");
		ImGui::Checkbox("Enable debugging of ray for last saved pixel", &m_Renderer.GetVisualDebugging().enableRaysDebugging);
		ImGui::Checkbox("Enable wireframe of triangles", &m_Renderer.GetVisualDebugging().enableWireframeTriangles);
		ImGui::Checkbox("Enable wireframe of BVH", &m_Renderer.GetVisualDebugging().enableWireframeBvh);

		if (ImGui::Button("Debug pixel"))
		{
			Debug();
		}

		ImGui::Separator();
		ImGui::Text("Settings");
		ImGui::Checkbox("Real time ray trace", &m_Renderer.GetSettings().enableRayTracing);
		ImGui::Checkbox("Enable shadows", &m_Renderer.GetSettings().enableShadows);
		ImGui::Checkbox("Apply texture(defaults to bilinear interpolation)", &m_Renderer.GetSettings().applyTexture);
		ImGui::SliderInt("Number of bounces(default is 1)", &m_Renderer.GetSettings().bounces, 0, 10);

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
		//m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		//m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
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

