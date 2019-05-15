//
// Created by efarhan on 5/3/19.
//

#include <engine.h>
#include <graphics.h>
#include <geometry.h>
#include <light.h>

#include <imgui.h>

#define IRRADIANCE_MAP
#define SPECULAR_IBL
//#define BG_IRRADIANCE_MAP

class HelloIBLDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
	void UpdateUi() override;
private:
	Sphere sphere = {};
	Shader pbrShader;
	Shader bgShader;

	float albedoColor[3] = { 1,0,1 };

	float ao = 1.0f;
	float lightIntensity = 300;
	PointLight pointLight[4];

	unsigned int captureFBO, captureRBO;
	Shader equirectToCubemapShader;
	unsigned envCubemap;
	unsigned int hdrTexture;
	Cube cubeMap = {};
#ifdef IRRADIANCE_MAP
	unsigned int irradianceMap;
	Shader irradianceShader;
#endif
#ifdef SPECULAR_IBL

	unsigned int prefilterMap;
#endif
};

void HelloIBLDrawingProgram::Init()
{
	programName = "Hello IBL";
	auto* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	auto& camera = engine->GetCamera();

	camera.Position = glm::vec3(0.0f, 0.0f, 5.0f);

	pbrShader.CompileSource(
		"shaders/27_hello_ibl/pbr.vert",
		"shaders/27_hello_ibl/pbr.frag");
	bgShader.CompileSource(
		"shaders/27_hello_ibl/backkground.vert",
		"shaders/27_hello_ibl/backkground.frag");
	equirectToCubemapShader.CompileSource(
		"shaders/27_hello_ibl/equirect.vert", 
		"shaders/27_hello_ibl/equirect.frag");
#ifdef IRRADIANCE_MAP
	irradianceShader.CompileSource(
		"shaders/27_hello_ibl/equirect.vert", 
		"shaders/27_hello_ibl/irradiance.frag");
#endif
	shaders.push_back(&pbrShader);
	shaders.push_back(&bgShader);
	shaders.push_back(&equirectToCubemapShader);

	pointLight[0].position = glm::vec3(10.0f, 10.0f, 10.0f);
	pointLight[1].position = glm::vec3(-10.0f, 10.0f, 10.0f);
	pointLight[2].position = glm::vec3(10.0f, 10.0f, -10.0f);
	pointLight[3].position = glm::vec3(10.0f, -10.0f, 10.0f);

	sphere.Init();
	cubeMap.Init();

	//capture
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
	//envCubemap
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		// note that we store each face with 16 bit floating point values
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
			512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	hdrTexture = stbCreateTexture("data/skybox/ridgecrest_road/Ridgecrest_Road_Ref.hdr");

	const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, 1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, 1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  -1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, 1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, 1.0f,  0.0f))
	};
	// convert HDR equirectangular environment map to cubemap equivalent
	equirectToCubemapShader.Bind();
	equirectToCubemapShader.SetInt("equirectangularMap", 0);
	equirectToCubemapShader.SetMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		equirectToCubemapShader.SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cubeMap.Draw(); // renders a 1x1 cube
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#ifdef IRRADIANCE_MAP
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0,
			GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
	irradianceShader.Bind();
	irradianceShader.SetInt("environmentMap", 0);
	irradianceShader.SetMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradianceShader.SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cubeMap.Draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
#ifdef SPECULAR_IBL
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
#endif

	glViewport(0, 0, config.screenWidth, config.screenHeight);
	
}

void HelloIBLDrawingProgram::Draw()
{
	ProcessInput();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	auto* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();

	
	glm::mat4 projection = glm::perspective(
		glm::radians(camera.Zoom),
		(float)config.screenWidth / (float)config.screenHeight,
		0.1f,
		1000.0f);

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
	pbrShader.SetInt("irradianceMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	pbrShader.SetFloat("ao", ao);
	pbrShader.SetVec3("camPos", camera.Position);
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(i-2.5f, j - 2.5f, 0));

			model = glm::scale(model, glm::vec3(0.25f));
			pbrShader.SetFloat("metallic", i / 5.0f);
			pbrShader.SetFloat("roughness", j / 5.0f);
			pbrShader.SetMat4("model", model);
			sphere.Draw();
		}
	}

	bgShader.Bind();
	bgShader.SetMat4("view", camera.GetViewMatrix());
	bgShader.SetMat4("projection", projection);
	bgShader.SetInt("environmentMap", 0);
	glActiveTexture(GL_TEXTURE0);
#ifdef BG_IRRADIANCE_MAP
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
#else
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
#endif
	cubeMap.Draw();
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
	auto& config = engine.GetConfiguration();
	config.screenWidth = 1280;
	config.screenHeight = 720;
	config.windowName = "Hello IBL";
	config.bgColor = { 0,0,0,0 };
	engine.AddDrawingProgram(new HelloIBLDrawingProgram());
	engine.Init();
	engine.GameLoop();
	return EXIT_SUCCESS;
}
