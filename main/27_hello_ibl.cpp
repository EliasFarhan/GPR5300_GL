//
// Created by efarhan on 5/3/19.
//

#include <engine.h>
#include <graphics.h>
#include <geometry.h>
#include <light.h>

#include <imgui.h>

class HelloIBLDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
	void UpdateUi() override;
private:
	Sphere sphere;
	Shader pbrShader;
	Shader pbrMapShader;

	float albedoColor[3] = { 1,0,1 };

	float ao = 1.0f;
	float lightIntensity = 300;
	PointLight pointLight[4];

	Skybox skybox;
	unsigned int captureFBO, captureRBO;
	Shader equirectangularToCubemapShader;
};

void HelloIBLDrawingProgram::Init()
{
	programName = "Hello PBR Basics";
	auto* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	auto& camera = engine->GetCamera();

	camera.Position = glm::vec3(0.0f, 0.0f, 5.0f);

	pbrShader.CompileSource(
		"shaders/25_hello_pbr/pbr.vert",
		"shaders/25_hello_pbr/pbr.frag");
	pbrMapShader.CompileSource(
		"shaders/25_hello_pbr/pbr.vert",
		"shaders/25_hello_pbr/pbr_map.frag");
	shaders.push_back(&pbrShader);
	shaders.push_back(&pbrMapShader);
	pointLight[0].position = glm::vec3(10.0f, 10.0f, 10.0f);
	pointLight[1].position = glm::vec3(-10.0f, 10.0f, 10.0f);
	pointLight[2].position = glm::vec3(10.0f, 10.0f, -10.0f);
	pointLight[3].position = glm::vec3(10.0f, -10.0f, 10.0f);

	sphere.Init();
	

	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
}

void HelloIBLDrawingProgram::Draw()
{
	ProcessInput();
	glEnable(GL_DEPTH_TEST);
	auto* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();

	glm::mat4 projection = glm::perspective(
		camera.Zoom,
		(float)config.screenWidth / config.screenHeight,
		0.1f, 100.0f);

	pbrShader.Bind();
	for (int i = 0; i < 4; i++)
	{
		pointLight[i].color = glm::vec3(1.0f)*lightIntensity;
		pointLight[i].Bind(pbrShader, i);
	}
	pbrShader.SetInt("pointLightsNmb", 4);
	pbrShader.SetMat4("view", camera.GetViewMatrix());
	pbrShader.SetMat4("projection", projection);
	pbrShader.SetVec3("albedo", albedoColor);

	pbrShader.SetFloat("ao", ao);
	pbrShader.SetVec3("camPos", camera.Position);
	glm::mat4 model;
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(i-2.5f, j - 2.5f, 0));

			model = glm::scale(model, glm::vec3(0.25f));
			pbrShader.SetFloat("metallic", i / 5.0f);
			pbrShader.SetFloat("roughness", j / 5.0f);
			pbrShader.SetMat4("model", model);
			sphere.Draw();
		}
	}

}

void HelloIBLDrawingProgram::Destroy()
{
}

void HelloIBLDrawingProgram::UpdateUi()
{
	ImGui::Separator();
	ImGui::ColorEdit3("albedo", albedoColor);
	ImGui::SliderFloat("lightColor", &lightIntensity, 1.0f, 300.0f);
	ImGui::SliderFloat("ao", &ao, 0.0f, 1.0f);
}


int main(int argc, char** argv)
{
	Engine engine;
	srand(0);
	auto& config = engine.GetConfiguration();
	config.screenWidth = 1280;
	config.screenHeight = 720;
	config.windowName = "Hello PBR";
	config.bgColor = { 0,0,0,0 };
	engine.AddDrawingProgram(new HelloIBLDrawingProgram());
	engine.Init();
	engine.GameLoop();
	return EXIT_SUCCESS;
}
