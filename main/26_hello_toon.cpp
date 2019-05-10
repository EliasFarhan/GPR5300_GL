#include <engine.h>
#include <graphics.h>
#include <model.h>
#include "geometry.h"
#include "light.h"
#include "imgui.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

class HelloToonDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
	void UpdateUi() override;
private:
	Model nanoModel;
	Model rockModel;
	Plane floorPlan;

	Shader toonShader;
	Shader modelShader;

	static const int maxLightNmb = 4;
	int lightNmb = 1;
	float lightIntensity = 1.0f;
	float lightDistance = 15.0f;
	int toonLayers = 4;
	PointLight lights[maxLightNmb] = {};
	glm::vec3 lightsPositions[maxLightNmb] = {};
	bool toonShading = true;
	bool rotatingLight = false;
};

void HelloToonDrawingProgram::Init()
{
	auto* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();

	programName = "Hello Toon";

	camera.Position = glm::vec3(0, 2, 5);

	nanoModel.Init("data/models/nanosuit2/nanosuit.obj");
	rockModel.Init("data/models/rocks01/rock_01.obj");
	modelShader.CompileSource("shaders/engine/model.vert", "shaders/engine/model.frag");
	toonShader.CompileSource("shaders/26_hello_toon/toon.vert", "shaders/26_hello_toon/toon.frag");
	lightsPositions[0] = glm::vec3(1, 0, 1);
	lightsPositions[1] = glm::vec3(-1, 0, -1);
	lightsPositions[2] = glm::vec3(1, 0, -1);
	lightsPositions[3] = glm::vec3(-1, 0, 1);
}

void HelloToonDrawingProgram::Draw()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	ProcessInput();
	auto* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();

	Shader& currentShader = toonShading ? toonShader : modelShader;
	const glm::mat4 projection = glm::perspective(
		camera.Zoom,
		(float)config.screenWidth / config.screenHeight,
		0.1f, 100.0f);
	const glm::mat4 view = camera.GetViewMatrix();

	currentShader.Bind();
	currentShader.SetMat4("projection", projection);
	currentShader.SetMat4("view", view);
	glm::mat4 modelMat = glm::mat4(1.0f);
	modelMat = glm::scale(modelMat, glm::vec3(0.2f, 0.2f, 0.2f));
	currentShader.SetMat4("model", modelMat);

	for(int i = 0; i < lightNmb;i++)
	{
		lights[i].intensity = lightIntensity;
		lights[i].distance = 2.0f*lightDistance;
		glm::vec3 newPos = lightsPositions[i];
		if(rotatingLight)
		{
			newPos = glm::rotate(newPos, static_cast<float>(engine->GetTimeSinceInit() / M_PI), glm::vec3(0, 1, 0));
		}
		lights[i].position = newPos * lightDistance;
		lights[i].Bind(currentShader, i);
	}
	currentShader.SetInt("pointLightsNmb", lightNmb);
	currentShader.SetInt("toonLayers", toonLayers);
	nanoModel.Draw(currentShader);

	modelMat = glm::mat4(1.0f);
	modelMat = glm::translate(modelMat, glm::vec3(3, 0, 0));
	modelMat = glm::scale(modelMat, glm::vec3(0.01f, 0.01f, 0.01f));
	currentShader.SetMat4("model", modelMat);
	rockModel.Draw(currentShader);
}

void HelloToonDrawingProgram::Destroy()
{
}

void HelloToonDrawingProgram::UpdateUi()
{
	ImGui::Checkbox("Toon Shading", &toonShading);
	ImGui::SliderFloat("Light Intensity", &lightIntensity, 0.5f, 10.0f);
	ImGui::Checkbox("Rotating Lights", &rotatingLight);
	ImGui::SliderInt("Toon Layers", &toonLayers, 2, 10);

}

int main(int argc, char** argv)
{
	Engine engine;
	auto& config = engine.GetConfiguration();
	config.screenWidth = 1024;
	config.screenHeight = 1024;
	config.windowName = "Hello Toon";
	config.bgColor = { 1,1,1,1 };
	engine.AddDrawingProgram(new HelloToonDrawingProgram());
	engine.Init();
	engine.GameLoop();
	return EXIT_SUCCESS;
}
