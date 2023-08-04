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

		Material greenSphere;
		greenSphere.Albedo = { 0.0f, 1.0f, 0.0f };
		greenSphere.Roughness = 1.0f;
		greenSphere.Metallic = 0.0f;
		{
			Mesh mesh;
			mesh.material = greenSphere;
			Vertex v1;
			v1.position = { 0.0f, -3.0f, 0.0f };
			v1.normal = { 0.0f, 0.0f, -1.0f };
			Vertex v2;
			v2.position = { 3.0f, -3.0f, 0.0f };
			v2.normal = { 0.57735026919f, 0.57735026919f,0.57735026919f };
			Vertex v3;
			v3.position = { 3.0f, 3.0f, 0.0f };
			v3.normal = { 0.0f, 0.0f, -1.0f };
			
			Vertex v4;
			v4.position = { 0.0f, 3.0f, 0.0f };
			//v4.normal = { 0.0f, 0.0f, -1.0f };
			v4.normal = { 0.0f, 0.0f, 1.0f };
			/*
			Vertex v5;
			v5.position = { 100.0f, 100.0f, 4.0f };
			v5.normal = { 0.0f, 0.0f, 1.0f };
			Vertex v6;
			v6.position = { 2.0f, 100.0f, 500.0f };
			v6.normal = { 0.0f, 1.0f, 0.0f };
			Vertex v7;
			v7.position = { 300.0f, 100.0f, 0.0f };
			v7.normal = { 1.0f, 0.0f, 0.0f };

			mesh.vertices = { v1, v2, v3, v4, v5, v6, v7 };
			mesh.triangles = { {0, 1, 2}, {0, 2, 3}, {1, 4, 6 }, {4, 5, 6}, {6, 2, 4 }, { 1, 2, 4 }, { 3, 4, 5 }};
			*/

			mesh.vertices = { v1, v2, v3, v4};
			mesh.triangles = { {0, 1, 2}, {0, 2, 3} };
			
			m_Scene.meshes.push_back(mesh);
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

