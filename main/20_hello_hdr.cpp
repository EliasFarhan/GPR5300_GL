#include <engine.h>
#include <graphics.h>
#include "light.h"
#include "geometry.h"
#include "math_utility.h"
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.inl>
#include "imgui.h"

class HelloHdrDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;

	void UpdateUi() override;
private:
	Shader hdrShader;
	Plane hdrPlane;
	unsigned hdrFBO = 0;
	unsigned rboDepth = 0;
	unsigned hdrColorBuffer = 0;

	Shader modelForwardShader;
	Plane corridorPlane;
	float corridorScale[3] = { 1.1f, 10.0f, 1.0f };
	unsigned corridorDiffuseMap = 0;
	unsigned corridorNormalMap = 0;
	unsigned corridorSpecularMap = 0;

	PointLight lights[5] = {};
	bool enableToneMapping = false;
	float exposure = 1.0f;
	float lightIntensity = 1.0f;
};

void HelloHdrDrawingProgram::Init()
{
	programName = "Hello HDR";
	auto* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	auto& camera = engine->GetCamera();

	camera.Position = glm::vec3(0.0f, 0.0f, 10.0f);
	
	corridorPlane.Init();
	corridorDiffuseMap = stbCreateTexture("data/sprites/bricks_02_dif.tga");
	corridorNormalMap = stbCreateTexture("data/sprites/bricks_02_nm.tga");
	corridorSpecularMap = stbCreateTexture("data/sprites/bricks_02_spec.tga");
	modelForwardShader.CompileSource(
		"shaders/20_hello_hdr/plane.vert", 
		"shaders/20_hello_hdr/plane.frag");
	shaders.push_back(&modelForwardShader);

	hdrPlane.Init();
	glGenFramebuffers(1, &hdrFBO);

	glGenTextures(1, &hdrColorBuffer);
	glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
		config.screenWidth, config.screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32,
		config.screenWidth,
		config.screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, hdrColorBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
		rboDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	hdrShader.CompileSource(
		"shaders/20_hello_hdr/hdr.vert", 
		"shaders/20_hello_hdr/hdr.frag");
	shaders.push_back(&hdrShader);

	for(int i = 0; i < 5;i++)
	{
		const glm::vec3 colors [5] = 
		{
			glm::vec3(1,0,0),
			glm::vec3(1,1,0),
			glm::vec3(0,1,0),
			glm::vec3(0,0,1),
			glm::vec3(0,1,1),

		};
		lights[i].position = glm::vec3(RandomRange(-128, 128) / 256.0f, RandomRange(-128, 128) / 256.0f, 7.5f-i*5);
		lights[i].color = colors[i];
		lights[i].intensity = 0.5f;
		lights[i].distance = 1.0f;

	}
	
}

void HelloHdrDrawingProgram::Draw()
{
	ProcessInput();
	auto* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Draw corridors
	modelForwardShader.Bind();
	for(int i = 0; i < 5; i++)
	{

		lights[i].intensity = lightIntensity;
		lights[i].Bind(modelForwardShader, i);
	}

	modelForwardShader.SetInt("pointLightsNmb", 5);
	const glm::mat4 projection = glm::perspective(
		camera.Zoom,
		(float)config.screenWidth / config.screenHeight,
		0.1f, 100.0f);
	modelForwardShader.SetMat4("projection", projection);
	modelForwardShader.SetMat4("view", camera.GetViewMatrix());
	modelForwardShader.SetVec2("texTiling", glm::vec2(corridorScale[0], corridorScale[1]));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, corridorDiffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, corridorNormalMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, corridorSpecularMap);
	modelForwardShader.SetInt("material.texture_diffuse1", 0);
	modelForwardShader.SetInt("material.texture_normal", 1);
	modelForwardShader.SetInt("material.texture_specular1", 2);
	modelForwardShader.SetVec3("viewPos", camera.Position);
	modelForwardShader.SetFloat("ambientIntensity", 0.0f);
	for (int i = 0; i < 4; i++)
	{
		
		const float angles[4] =
		{
			0.0f,
			90.0f,
			180.0f,
			-90.0f
		};
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, glm::vec3(0, -1, -1));
		model = glm::scale(model, glm::vec3(corridorScale[0], corridorScale[1], corridorScale[2]));
		auto quaternion = glm::quat(glm::vec3(glm::radians(90.0f), 0, glm::radians(angles[i])));
		model = glm::mat4_cast(quaternion)*model;
		

		modelForwardShader.SetMat4("model", model);
		corridorPlane.Draw();
	}

	//Show hdr quad
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	hdrShader.Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
	hdrShader.SetInt("hdrBuffer", 0);
	hdrShader.SetBool("toneMapping", enableToneMapping);
	hdrShader.SetFloat("exposure", exposure);
	hdrPlane.Draw();
	
}

void HelloHdrDrawingProgram::Destroy()
{
}

void HelloHdrDrawingProgram::UpdateUi()
{
	Engine* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	ImGui::Separator();
	ImGui::InputFloat3("Corridor Scale", corridorScale);
	ImGui::InputFloat3("Camera Position", (float*)&camera.Position);
	ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f);
	ImGui::Checkbox("Enable Tone Mapping", &enableToneMapping);
	ImGui::SliderFloat("Backlight Intensity", &lightIntensity, 1.0f, 500.0f);
}

int main(int argc, char** argv)
{
	Engine engine;
	srand(0);
	auto& config = engine.GetConfiguration();
	config.screenWidth = 1024;
	config.screenHeight = 1024;
	config.windowName = "Hello HDR";
	config.bgColor = { 1,1,1,1 };
	engine.AddDrawingProgram(new HelloHdrDrawingProgram());
	engine.Init();
	engine.GameLoop();
	return EXIT_SUCCESS;
}
