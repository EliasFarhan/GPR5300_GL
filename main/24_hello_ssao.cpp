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

#include <random>

class HelloSSAODrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;

	void UpdateUi() override;
private:

	void DrawScene();

	Shader hdrShader;
	Plane hdrPlane;
	unsigned hdrFBO = 0;
	unsigned rboDepth = 0;
	unsigned hdrColorBuffer[2];

	Shader modelDeferredShader;

	Plane corridorPlane;
	float corridorScale[3] = { 5.0f, 5.0f, 1.0f };
	unsigned corridorDiffuseMap = 0;
	unsigned corridorNormalMap = 0;
	unsigned corridorSpecularMap = 0;
	Model model;


	Cube lightCube;
	Shader lightShader;


	static const int maxLightNmb = 128;
	int lightNmb = 1;
	PointLight lights[maxLightNmb] = {};
	float exposure = 0.1f;
	float lightIntensity = 1.0f;
	//blur
	GLuint pingpongFBO[2];
	GLuint pingpongBuffer[2];
	Shader blurShader;


	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gColorSpec, gAlbedoSpec, gSSAOAlbedo;
	unsigned int gRBO;
	Shader lightingPassShader;

	unsigned ssaoFBO;
	unsigned ssaoColorBuffer;
	Shader ssaoPassShader;

	unsigned ssaoBlurFBO;
	unsigned ssaoColorBufferBlur;
	Shader ssaoBlurPassShader;
	std::vector<glm::vec3> ssaoKernel;
	unsigned noiseTexture;

	glm::mat4 projection;
	float ssaoRadius = 0.5f;
	int kernelSize = 64;
};

void HelloSSAODrawingProgram::Init()
{
	
	programName = "Hello SSAO";
	auto* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	auto& camera = engine->GetCamera();

	camera.Position = glm::vec3(0.0f, 2.0f, 2.0f);

	model.Init("data/models/nanosuit2/nanosuit.obj");
	corridorPlane.Init();
	corridorDiffuseMap = stbCreateTexture("data/sprites/bricks_02_dif.tga");
	corridorNormalMap = stbCreateTexture("data/sprites/bricks_02_nm.tga");
	corridorSpecularMap = stbCreateTexture("data/sprites/bricks_02_spec.tga");



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

	for (int i = 0; i < maxLightNmb; i++)
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
		"shaders/24_hello_ssao/ssao_basic.vert",
		"shaders/24_hello_ssao/ssao_basic.frag");
	lightingPassShader.CompileSource(
		"shaders/24_hello_ssao/lighting_pass.vert",
		"shaders/24_hello_ssao/lighting_pass.frag");
	//deferred Framebuffer


	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// - position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, config.screenWidth, config.screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		config.screenWidth, config.screenHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// - ssao color buffer
	glGenTextures(1, &gSSAOAlbedo);
	glBindTexture(GL_TEXTURE_2D, gSSAOAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
		config.screenWidth, config.screenHeight, 0, 
		GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gSSAOAlbedo, 0);
	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int gAttachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, gAttachments);
	glGenRenderbuffers(1, &gRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, gRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32,
		config.screenWidth,
		config.screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
		gRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Ambient occlusion
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, config.screenWidth, config.screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ssaoPassShader.CompileSource(
			"shaders/24_hello_ssao/ssao_pass.vert",
			"shaders/24_hello_ssao/ssao_pass.frag");

	

	// generate noise texture
	// ----------------------
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//Ambient occlusion blur
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, config.screenWidth, config.screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ssaoBlurPassShader.CompileSource(
			"shaders/24_hello_ssao/ssao_blur.vert",
			"shaders/24_hello_ssao/ssao_blur.frag");
}



void HelloSSAODrawingProgram::Draw()
{

	rmt_ScopedCPUSample(HelloSSAO_CPU, 0);
	rmt_ScopedOpenGLSample(HelloSSAO_GPU);
	rmt_BeginOpenGLSample(G_Buffer);
	ProcessInput();
	auto* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();
	projection = glm::perspective(
			camera.Zoom,
			(float)config.screenWidth / config.screenHeight,
			0.1f, 100.0f);
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	DrawScene();
	rmt_EndOpenGLSample(); //GBUFFER
	//Ambient occlusion pass
	rmt_BeginOpenGLSample(AO_Pass);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	{
		glClear(GL_COLOR_BUFFER_BIT);
		ssaoPassShader.Bind();
		// generate sample kernel
	// ----------------------
		ssaoKernel.clear();
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
		std::default_random_engine generator;

		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = float(i) / kernelSize;

			// scale samples s.t. they're more aligned to center of kernel
			scale = 0.1f + scale * scale * (1.0f - 0.1f); //a + f * (b - a);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}
		// Send kernel + rotation 
		for (unsigned int i = 0; i < kernelSize; ++i)
			ssaoPassShader.SetVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
		ssaoPassShader.SetInt("kernelSize", kernelSize);
		ssaoPassShader.SetFloat("radius", ssaoRadius);
		ssaoPassShader.SetMat4("projection", projection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		ssaoPassShader.SetInt("gPosition", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);

		ssaoPassShader.SetInt("gNormal", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);

		ssaoPassShader.SetInt("texNoise", 2);
		ssaoPassShader.SetVec2("noiseScale", glm::vec2(config.screenWidth / 4.0f, config.screenHeight / 4.0f));
		hdrPlane.Draw(); 
	}
	rmt_EndOpenGLSample();
	rmt_BeginOpenGLSample(SSAO_Blur);
	// Blur SSAO texture to remove noise
		// ------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	ssaoBlurPassShader.Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	hdrPlane.Draw();

	rmt_EndOpenGLSample();
	rmt_BeginOpenGLSample(LightPass);
	//Light pass
	{
		

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//copy depth buffer into hdrFBO
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(
			0, 0, config.screenWidth, config.screenHeight, 0, 0, config.screenWidth, config.screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST
		);
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
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		lightingPassShader.SetInt("gPosition", 0);
		lightingPassShader.SetInt("gNormal", 1);
		lightingPassShader.SetInt("gAlbedoSpec", 2);
		lightingPassShader.SetInt("gSsaoAlbedo", 3);
		lightingPassShader.SetInt("pointLightsNmb", lightNmb);
		lightingPassShader.SetVec3("viewPos", camera.Position);
		//render light in forward rendering
		hdrPlane.Draw();
		glEnable(GL_DEPTH_TEST);
	}
	lightShader.Bind();

	rmt_EndOpenGLSample();


	
	

}

void HelloSSAODrawingProgram::Destroy()
{
}

void HelloSSAODrawingProgram::UpdateUi()
{
	Engine* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	ImGui::Separator();
	ImGui::SliderFloat("SSAO Radius", &ssaoRadius, 0.1f, 1.0f);
	ImGui::InputFloat3("Corridor Scale", corridorScale);
	ImGui::InputFloat3("Camera Position", (float*)&camera.Position);
	ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f);
	ImGui::SliderFloat("Backlight Intensity", &lightIntensity, 0.1f, 500.0f);
	ImGui::SliderInt("Lights Nmb", &lightNmb, 0, maxLightNmb);
	ImGui::SliderInt("Kernel Size", &kernelSize, 1, 64);
}
void HelloSSAODrawingProgram::DrawScene()
{
	auto* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();



	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	Shader& currentShader = modelDeferredShader;
	//Draw corridors
	currentShader.Bind();
	currentShader.SetInt("pointLightsNmb", lightNmb);

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
	currentShader.SetFloat("material.shininess", 16.0f);
	currentShader.SetVec3("viewPos", camera.Position);
	currentShader.SetFloat("ambientIntensity", 0.0f);
	
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(0, 0, -2));
	model = glm::scale(model, glm::vec3(corridorScale[0], corridorScale[1], corridorScale[2]));


	currentShader.SetMat4("model", model);
	corridorPlane.Draw();

	

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(
			0.0f,
			0.0f,
			0.0f));
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
	currentShader.SetVec2("texTiling", glm::vec2(1.0f, 1.0f));
	currentShader.SetMat4("model", model);
	this->model.Draw(currentShader);

}

int main(int argc, char** argv)
{
	Engine engine;
	srand(0);
	auto& config = engine.GetConfiguration();
	config.screenWidth = 1280;
	config.screenHeight = 720;
	config.windowName = "Hello SSAO";
	config.bgColor = { 1,1,1,1 };
	engine.AddDrawingProgram(new HelloSSAODrawingProgram());
	engine.Init();
	engine.GameLoop();
	return EXIT_SUCCESS;
}
