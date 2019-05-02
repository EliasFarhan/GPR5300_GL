#include <engine.h>
#include <graphics.h>
#include <geometry.h>
#include <model.h>
#include "light.h"
#include "imgui.h"

class HelloShadowDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
	void UpdateUi() override;
private:
	void DrawScene(Shader& currentShader);
	Shader depthShader;
	Shader modelShader;
	Shader gammaShader;

	Model floorPlane;
	Model humanModel;
	Model rockModel;
	Cube cube;
	unsigned int cubeTexture;
	unsigned int cubeSpecularTexture;
	Plane postProcessingPlane;
	unsigned int postProcessingFBO;
	unsigned int postProcessingRBO;
	unsigned int postProcessingTexture;

	DirectionLight directionLight;
	unsigned int floorTexture;

	unsigned int depthMapFBO;
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthMap;
	bool shadowBiasEnable = false;
	bool clampDepthMap = false;
	bool enableFaceCulling = false;
	bool enablePcf = false;
	bool enableGammaCorrection = false;
};

void HelloShadowDrawingProgram::Init()
{
	programName = "Hello Shadow";
	auto* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	auto& camera = engine->GetCamera();

	camera.Position = glm::vec3(0.0f, 2.0f, 10.0f);

	floorPlane.Init("data/models/granit01/granit.obj");
	floorTexture = stbCreateTexture("data/models/granit01/granite_01_dif.tga");
	humanModel.Init("data/models/nanosuit2/nanosuit.obj");
	rockModel.Init("data/models/rocks01/rock_01.obj");
	cube.Init();
	cubeTexture = stbCreateTexture("data/sprites/container2.png");
	cubeSpecularTexture = stbCreateTexture("data/sprites/container2_specular.png");

	depthShader.CompileSource(
		"shaders/engine/depth.vert", 
		"shaders/engine/depth.frag");
	modelShader.CompileSource(
		"shaders/21_hello_shadow/model.vert",
		"shaders/21_hello_shadow/model.frag");
	gammaShader.CompileSource(
		"shaders/21_hello_shadow/gamma.vert", 
		"shaders/21_hello_shadow/gamma.frag");
	shaders.push_back(&modelShader);
	shaders.push_back(&depthShader);
	shaders.push_back(&gammaShader);
	


	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	postProcessingPlane.Init();
	glGenFramebuffers(1, &postProcessingFBO);
	glGenTextures(1, &postProcessingTexture);
	glBindTexture(GL_TEXTURE_2D, postProcessingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
		config.screenWidth, config.screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, postProcessingFBO);
	glGenRenderbuffers(1, &postProcessingRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, postProcessingRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32,
		config.screenWidth, 
		config.screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, postProcessingTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 
		postProcessingRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	directionLight.enable = true;
	directionLight.direction = glm::vec3(1.0f, -1.0f, 0.0f);
	directionLight.position = glm::vec3(-10.0f, 10.0f, 0.0f);
	directionLight.intensity = 1.0f;
}

void HelloShadowDrawingProgram::Draw()
{
	auto* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();

	ProcessInput();

	glEnable(GL_DEPTH_TEST);
	if (enableFaceCulling)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	float near_plane = 1.0f, far_plane = 20.0f;
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 
		near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(
		directionLight.position,
		directionLight.position+directionLight.direction,
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;
	depthShader.Bind();
	depthShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
	if (enableFaceCulling)
	{
		glCullFace(GL_FRONT);
	}
	DrawScene(depthShader);
	if (enableFaceCulling)
	{
		glCullFace(GL_BACK);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, postProcessingFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, config.screenWidth, config.screenHeight);
	modelShader.Bind();
	modelShader.SetBool("shadowBiasEnable", shadowBiasEnable);
	modelShader.SetBool("pcf", enablePcf);
	modelShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
	modelShader.SetInt("shadowMap", 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawScene(modelShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	gammaShader.Bind();
	glDisable(GL_DEPTH_TEST);
	gammaShader.SetInt("screenTexture", 0);
	gammaShader.SetBool("gammaCorrection", &enableGammaCorrection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, postProcessingTexture);
	postProcessingPlane.Draw();
}

void HelloShadowDrawingProgram::Destroy()
{
}

void HelloShadowDrawingProgram::UpdateUi()
{
	ImGui::Separator();
	ImGui::Checkbox("Shadow Bias", &shadowBiasEnable);
	ImGui::Checkbox("Enable face culling", &enableFaceCulling);
	ImGui::Checkbox("Enable PCF", &enablePcf);
	ImGui::Checkbox("Enable Gamma Correction", &enableGammaCorrection);
}

void HelloShadowDrawingProgram::DrawScene(Shader& currentShader)
{
	auto* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();

	glm::mat4 projection = glm::perspective(
		camera.Zoom,
		(float)config.screenWidth / config.screenHeight,
		0.1f, 100.0f);
	currentShader.SetMat4("projection", projection);
	currentShader.SetMat4("view", camera.GetViewMatrix());

	currentShader.SetBool("directionalLightEnable", true);
	directionLight.Bind(currentShader, 0);
	glm::mat4 modelMat = glm::mat4(1.0f);
	modelMat = glm::scale(modelMat, glm::vec3(0.2f, 0.2f, 0.2f));
	modelMat = glm::rotate(modelMat, glm::radians(10.0f), glm::vec3(0, 1, 0));
	currentShader.SetMat4("model", modelMat);
	humanModel.Draw(currentShader);

	modelMat = glm::mat4(1.0f);
	modelMat = glm::translate(modelMat, glm::vec3(2.0f, 1.0f, 0.5f));
	modelMat = glm::scale(modelMat, glm::vec3(0.01f, 0.01f, 0.01f));
	currentShader.SetMat4("model", modelMat);
	rockModel.Draw(currentShader);

	modelMat = glm::mat4(1.0f);
	modelMat = glm::scale(modelMat, glm::vec3(10.0f, 10.0f, 10.0f));
	currentShader.SetMat4("model", modelMat);
	currentShader.SetFloat("texTiling", 10.0f);
	floorPlane.Draw(currentShader);
	currentShader.SetFloat("texTiling", 1.0f);
	modelMat = glm::mat4(1.0f);
	modelMat = glm::translate(modelMat, glm::vec3(-4.0f,1.0f,2.0f));
	modelMat = glm::rotate(modelMat, glm::radians(35.0f), glm::vec3(1.0f,1.0f,0.0f));
	currentShader.SetMat4("model", modelMat);
	currentShader.SetInt("material.texture_diffuse1", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cubeTexture);
	currentShader.SetInt("material.texture_specular1", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cubeSpecularTexture);
	cube.Draw();
}

int main(int argc, char** argv)
{
	Engine engine;

	auto& config = engine.GetConfiguration();
	config.screenWidth = 1024;
	config.screenHeight = 1024;
	config.windowName = "Hello Shadow";
	config.bgColor = { 1,1,1,1 };

	engine.AddDrawingProgram(new HelloShadowDrawingProgram());

	engine.Init();
	engine.GameLoop();

	return EXIT_SUCCESS;
}
