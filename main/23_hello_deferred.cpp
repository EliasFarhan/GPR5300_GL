#include <engine.h>
#include <graphics.h>
#include "light.h"
#include "geometry.h"
#include <model.h>
#include "math_utility.h"
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.inl>
#include "imgui.h"
#include <Remotery.h>

class HelloDeferredDrawingProgram : public DrawingProgram
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
	unsigned hdrColorBuffer[2];

	Shader modelForwardShader;
	Shader modelDeferredShader;
	
	Plane corridorPlane;
	float corridorScale[3] = { 1.1f, 100.0f, 1.0f };
	unsigned corridorDiffuseMap = 0;
	unsigned corridorNormalMap = 0;
	unsigned corridorSpecularMap = 0;
	Model model;


	Cube lightCube;
	Shader lightShader;
	const int modelNmb = 10;
	static const int lightNmb = 128;
	PointLight lights[lightNmb] = {};
	float exposure = 1.0f;
	float lightIntensity = 1.0f;
	//blur
	GLuint pingpongFBO[2];
	GLuint pingpongBuffer[2];
	Shader blurShader;

	
	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gColorSpec, gAlbedoSpec;
	unsigned int gRBO;
	Shader lightingPassShader;
	bool deferred = true;
};

void HelloDeferredDrawingProgram::Init()
{
	programName = "Hello Deferred";
	auto* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	auto& camera = engine->GetCamera();

	camera.Position = glm::vec3(0.0f, 0.0f, 10.0f);

	model.Init("data/models/nanosuit2/nanosuit.obj");
	corridorPlane.Init();
	corridorDiffuseMap = stbCreateTexture("data/sprites/bricks_02_dif.tga");
	corridorNormalMap = stbCreateTexture("data/sprites/bricks_02_nm.tga");
	corridorSpecularMap = stbCreateTexture("data/sprites/bricks_02_spec.tga");
	modelForwardShader.CompileSource(
		"shaders/22_hello_bloom/plane.vert",
		"shaders/22_hello_bloom/plane.frag");
	shaders.push_back(&modelForwardShader);



	hdrPlane.Init();
	glGenFramebuffers(1, &hdrFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glGenTextures(2, hdrColorBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, hdrColorBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
			config.screenWidth, config.screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
			GL_TEXTURE_2D, hdrColorBuffer[i], 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32,
		config.screenWidth,
		config.screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
		rboDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	hdrShader.CompileSource(
		"shaders/22_hello_bloom/hdr.vert",
		"shaders/22_hello_bloom/hdr.frag");
	shaders.push_back(&hdrShader);

	for (int i = 0; i < lightNmb; i++)
	{

		lights[i].position = glm::vec3(
			RandomRange(-128, 128) / 256.0f,
			RandomRange(-128, 128) / 256.0f,
			7.5f - (float)(i*50.0f) / lightNmb);
		lights[i].color = glm::vec3(
			RandomRange(0, 256) / 256.0f,
			RandomRange(0, 256) / 256.0f,
			RandomRange(0, 256) / 256.0f
		);
		lights[i].intensity = 0.3f;
		lights[i].distance = 1.0f;
	}
	lightCube.Init();
	lightShader.CompileSource("shaders/22_hello_bloom/light.vert", "shaders/22_hello_bloom/light.frag");

	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB16F, config.screenWidth, config.screenHeight, 0, GL_RGB, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
		);
	}
	blurShader.CompileSource("shaders/22_hello_bloom/hdr.vert", "shaders/22_hello_bloom/blur.frag");
	
	modelDeferredShader.CompileSource(
		"shaders/23_hello_deferred/deferred.vert",
		"shaders/23_hello_deferred/deferred.frag");
	lightingPassShader.CompileSource(
		"shaders/23_hello_deferred/lighting_pass.vert",
		"shaders/23_hello_deferred/lighting_pass.frag");
	//deferred Framebuffer


	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// - position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, config.screenWidth,config.screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// - normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, config.screenWidth, config.screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// - color + specular color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, config.screenWidth, config.screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int gAttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, gAttachments);
	glGenRenderbuffers(1, &gRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, gRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32,
		config.screenWidth,
		config.screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
		gRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HelloDeferredDrawingProgram::Draw()
{
	ProcessInput();
	auto* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, deferred ? gBuffer : hdrFBO);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Shader& currentShader = deferred ? modelDeferredShader : modelForwardShader;
	//Draw corridors
	currentShader.Bind();
	if (!deferred)
	{
		for (int i = 0; i < lightNmb; i++)
		{
			lights[i].intensity = lightIntensity;
			lights[i].Bind(currentShader, i);
		}
	}

	currentShader.SetInt("pointLightsNmb", 5);
	const glm::mat4 projection = glm::perspective(
		camera.Zoom,
		(float)config.screenWidth / config.screenHeight,
		0.1f, 100.0f);
	currentShader.SetMat4("projection", projection);
	currentShader.SetMat4("view", camera.GetViewMatrix());
	currentShader.SetVec2("texTiling", glm::vec2(corridorScale[0], corridorScale[1]));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, corridorDiffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, corridorNormalMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, corridorSpecularMap);
	currentShader.SetInt("material.texture_diffuse1", 0);
	currentShader.SetInt("material.texture_normal", 1);
	currentShader.SetInt("material.texture_specular1", 2);
	currentShader.SetVec3("viewPos", camera.Position);
	currentShader.SetFloat("ambientIntensity", 0.0f);
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


		currentShader.SetMat4("model", model);
		corridorPlane.Draw();
	}
	for(int i = 0; i < modelNmb;i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(
			0.0f, 
			0.0f, 
			7.5f - (i*50.0f) / modelNmb));
		model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
		currentShader.SetVec2("texTiling", glm::vec2(1.0f, 1.0f));
		currentShader.SetMat4("model", model);
		this->model.Draw(currentShader);
	}
	if(deferred)
	{
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lightingPassShader.Bind();

		for (int i = 0; i < lightNmb; i++)
		{
			lights[i].intensity = lightIntensity;
			lights[i].Bind(lightingPassShader, i);
		}
		//light pass
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		lightingPassShader.SetInt("gPosition", 0);
		lightingPassShader.SetInt("gNormal", 1);
		lightingPassShader.SetInt("gAlbedoSpec", 2);
		lightingPassShader.SetInt("pointLightsNmb", lightNmb);
		lightingPassShader.SetVec3("viewPos", camera.Position);
		//render light in forward rendering
		hdrPlane.Draw();
		glEnable(GL_DEPTH_TEST);
	}
	lightShader.Bind();
	//Draw lights
	/*for (auto& light : lights)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, light.position);
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		lightShader.SetVec3("lightColor", light.color);
		lightShader.SetMat4("model", model);
		lightShader.SetMat4("view", camera.GetViewMatrix());
		lightShader.SetMat4("projection", projection);
		lightCube.Draw();
	}*/
	
	bool horizontal = true, first_iteration = true;
	int amount = 10;
	blurShader.Bind();
	for (unsigned int i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
		blurShader.SetInt("horizontal", horizontal);
		blurShader.SetInt("image", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(
			GL_TEXTURE_2D, first_iteration ? hdrColorBuffer[1] : pingpongBuffer[!horizontal]
		);
		hdrPlane.Draw();
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//Show hdr quad
	hdrShader.Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrColorBuffer[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
	hdrShader.SetInt("hdrBuffer", 0);
	hdrShader.SetInt("bloomBlur", 1);
	hdrShader.SetFloat("exposure", exposure);
	hdrPlane.Draw();

}

void HelloDeferredDrawingProgram::Destroy()
{
}

void HelloDeferredDrawingProgram::UpdateUi()
{
	Engine* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	ImGui::Separator();
	ImGui::Checkbox("Deferred", &deferred);
	ImGui::InputFloat3("Corridor Scale", corridorScale);
	ImGui::InputFloat3("Camera Position", (float*)&camera.Position);
	ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f);
	ImGui::SliderFloat("Backlight Intensity", &lightIntensity, 0.1f, 500.0f);
}

int main(int argc, char** argv)
{
	Engine engine;
	srand(0);
	auto& config = engine.GetConfiguration();
	config.screenWidth = 1024;
	config.screenHeight = 1024;
	config.windowName = "Hello Deferred";
	config.bgColor = { 1,1,1,1 };
	engine.AddDrawingProgram(new HelloDeferredDrawingProgram());
	engine.Init();
	engine.GameLoop();
	return EXIT_SUCCESS;
}
