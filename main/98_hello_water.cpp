#include <map>
#include <vector>

#include <engine.h>
#include <graphics.h>
#include <camera.h>
#include <model.h>

#include <Remotery.h>
#include "file_utility.h"

#include <json.hpp>
using json = nlohmann::json;
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>

#include "imgui.h"

class SceneDrawingProgram;

//#define DEBUG_WATA_REFLECTION
#define REFLECTION_MAP
class HelloWaterDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
	void UpdateUi() override;
private:
	Camera underWaterCamera;
	Camera* sceneCamera = nullptr;
	SceneDrawingProgram* scene = nullptr;

	Shader basicShader; //just to render to the stencil buffer

	

	float floorVertices[6 * 5] = 
	{
		-1.0f, 0.0f, -1.f, 0.0f, 0.0f,
		 1.0f, 0.0f, -1.f, 1.0f, 0.0f,
		 1.0f,  0.0f, 1.f, 1.0f, 1.0f,
		 1.0f,  0.0f, 1.f, 1.0f, 1.0f,
		-1.0f,  0.0f, 1.f, 0.0f, 1.0f,
		-1.0f, 0.0f, -1.f, 0.0f, 0.0f,
	};
	GLuint waterVAO;
	GLuint waterVBO;
	float waterPosition[3]{0,0,0};
#ifdef REFLECTION_MAP
	Shader underWaterShader;
	unsigned int texColorBuffer;
	unsigned int framebuffer;
	unsigned int rbo;
	Shader frameBufferShaderProgram;
	float quadVertices[4 * 6] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  1.0f, 1.0f,
		-1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f,  0.0f, 0.0f,

		-1.0f,  1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f,  1.0f,  0.0f, 1.0f
	};
	unsigned int quadVAO, quadVBO;

#endif
};

class SceneDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
	void UpdateUi() override;
	//Getters for Water Drawing Program
	Camera* GetCamera() { return &camera; }
	glm::mat4& GetProjection() { return projection; }
	std::vector<Model*>& GetModels() { return models; }
	std::vector<glm::vec3>& GetPositions() { return positions; }
	std::vector<glm::vec3>& GetScales() { return scales; }
	std::vector<glm::vec3>& GetRotations() { return rotations; }
	size_t GetModelNmb() { return modelNmb; }
	Skybox& GetSkybox() { return skybox; }
protected:

	void ProcessInput();
	Skybox skybox;
	Shader modelShader;
	Camera camera = Camera(glm::vec3(0.0f, 3.0f, 10.0f));
	glm::mat4 projection = {};
	const char* jsonPath = "scenes/water.json";
	json sceneJson;
	std::map<std::string, Model> modelMap;
	std::vector<Model*> models;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> scales;
	std::vector<glm::vec3> rotations;
	size_t modelNmb;

	float lastX = 0;
	float lastY = 0;

};


void HelloWaterDrawingProgram::Init()
{
	programName = "Under Water Drawing";
	Engine* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	auto& drawingPrograms = engine->GetDrawingPrograms();

	scene = dynamic_cast<SceneDrawingProgram*>(drawingPrograms[0]);
	sceneCamera = scene->GetCamera();
	basicShader.CompileSource(
		"shaders/engine/basic.vert",
		"shaders/engine/basic.frag");
	shaders.push_back(&basicShader);
	glGenVertexArrays(1, &waterVAO);
	glGenBuffers(1, &waterVBO);
	//bind water quad (need tex coords for the final rendering
	glBindVertexArray(waterVAO);

	glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// tex coords attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

#ifdef REFLECTION_MAP
	underWaterShader.CompileSource(
		"shaders/98_hello_water/water_scene.vert", 
		"shaders/98_hello_water/water_scene.frag");
	shaders.push_back(&underWaterShader);
	
	//Generate under water refelction map framebuffer
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, config.screenWidth, config.screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, config.screenWidth, config.screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	frameBufferShaderProgram.CompileSource(
		"shaders/15_hello_post_processing/frame.vert",
		"shaders/15_hello_post_processing/frame.frag"
	);
	
	//screen quad
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);

#endif

}

void HelloWaterDrawingProgram::Draw()
{
	rmt_ScopedOpenGLSample(DrawWater);
	rmt_ScopedCPUSample(DrawWaterCPU, 0);
	Engine* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();


	//Invert Camera
	underWaterCamera.Front = glm::reflect(sceneCamera->Front, glm::vec3(0.0f, 1.0f, 0.0f));
	underWaterCamera.Up = glm::reflect(sceneCamera->Up, glm::vec3(0.0f, 1.0f, 0.0f));
	underWaterCamera.Position = sceneCamera->Position;
	underWaterCamera.Position.y = underWaterCamera.Position.y-2*abs(underWaterCamera.Position.y-waterPosition[1]);
#ifdef REFLECTION_MAP
	//Under Water framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Stencil pass
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0xFF); // Write to stencil buffer
	glClear(GL_STENCIL_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//Draw water plane on stencil buffer
	basicShader.Bind();
	glm::mat4 stencilWaterModel = glm::mat4(1.0f);
	stencilWaterModel = glm::translate(stencilWaterModel, glm::vec3(waterPosition[0], waterPosition[1], waterPosition[2]));

	stencilWaterModel = glm::scale(stencilWaterModel, glm::vec3(5.0f, 5.0f, 5.0f));

	basicShader.SetMat4("model", stencilWaterModel);
	basicShader.SetMat4("view", underWaterCamera.GetViewMatrix());
	basicShader.SetMat4("projection", scene->GetProjection());
	
	glBindVertexArray(waterVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
	glStencilMask(0x00); //Not reading stencil anymore
	//Draw scene on top of water in reflection map
	underWaterShader.Bind();
	underWaterShader.SetFloat("waterHeight", waterPosition[1]);
	underWaterShader.SetMat4("projection", scene->GetProjection());
	underWaterShader.SetMat4("view", underWaterCamera.GetViewMatrix());
	for(auto i = 0l; i < scene->GetModelNmb();i++)
	{
		glm::mat4 modelMatrix(1.0f);
		modelMatrix = glm::translate(modelMatrix, scene->GetPositions()[i]);
		modelMatrix = glm::scale(modelMatrix, scene->GetScales()[i]);
		glm::quat quaternion = glm::quat(scene->GetRotations()[i]);
		modelMatrix = glm::mat4_cast(quaternion)*modelMatrix;

		underWaterShader.SetMat4("model", modelMatrix);

		scene->GetModels()[i]->Draw(underWaterShader);
	}
	Skybox& skybox = scene->GetSkybox();
	const auto underWaterView = underWaterCamera.GetViewMatrix();
	skybox.SetViewMatrix(underWaterView);
	skybox.SetProjectionMatrix(scene->GetProjection());
	skybox.Draw();
	glDisable(GL_STENCIL_TEST);
	//Draw the water with the texture generated by the reflection map
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default screen framebuffer
	glClearColor(config.bgColor.r, config.bgColor.g, config.bgColor.b, 1.0f);

#ifdef DEBUG_WATA_REFLECTION
	frameBufferShaderProgram.Bind();

	frameBufferShaderProgram.SetInt("screenTexture", 0);
	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
#else
	//stencil pass on default framebuffer
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0xFF); // Write to stencil buffer
	glClear(GL_STENCIL_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//Draw water plane on stencil buffer
	basicShader.Bind();

	basicShader.SetMat4("model", stencilWaterModel);
	basicShader.SetMat4("view", sceneCamera->GetViewMatrix());
	basicShader.SetMat4("projection", scene->GetProjection());

	glBindVertexArray(waterVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
	glStencilMask(0x00); //Not reading stencil anymore
	frameBufferShaderProgram.Bind();

	frameBufferShaderProgram.SetInt("screenTexture", 0);
	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_STENCIL_TEST);
#endif
#endif
#ifndef DEBUG_WATA_REFLECTION
#ifndef REFLECTION_MAP
	basicShader.Bind();
	glm::mat4 waterModel = glm::mat4(1.0f);
	waterModel = glm::translate(waterModel, glm::vec3(waterPosition[0], waterPosition[1], waterPosition[2]));
	waterModel = glm::scale(waterModel, glm::vec3(5.0f, 5.0f, 5.0f));

	basicShader.SetMat4("model", waterModel);
	basicShader.SetMat4("view", sceneCamera->GetViewMatrix());
	basicShader.SetMat4("projection", scene->GetProjection());

	glBindVertexArray(waterVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
#endif
#endif
}

void HelloWaterDrawingProgram::Destroy()
{
}

void HelloWaterDrawingProgram::UpdateUi()
{
	ImGui::Separator();
	ImGui::InputFloat3("position", waterPosition);
}

void SceneDrawingProgram::Init()
{
	programName = "Scene";

	modelShader.CompileSource(
		"shaders/engine/model.vert", 
		"shaders/engine/model.frag");
	shaders.push_back(&modelShader);
	const auto jsonText = LoadFile(jsonPath);
	sceneJson = json::parse(jsonText);
	modelNmb = sceneJson["models"].size();
	models.resize(modelNmb);
	positions.resize(modelNmb);
	scales.resize(modelNmb);
	rotations.resize(modelNmb);
	int i = 0;
	for(auto& model : sceneJson["models"])
	{
		const std::string modelName = model["model"];
		if(modelMap.find(modelName) == modelMap.end())
		{
			//Load model
			modelMap[modelName] = Model();
			modelMap[modelName].Init(modelName.c_str());
		}
		models[i] = &modelMap[modelName];
		
		glm::vec3 position;
		position.x = model["position"][0];
		position.y = model["position"][1];
		position.z = model["position"][2];
		positions[i] = position;

		glm::vec3 scale;
		scale.x = model["scale"][0];
		scale.y = model["scale"][1];
		scale.z = model["scale"][2];
		scales[i] = scale;

		glm::vec3 rotation;
		rotation.x = model["angles"][0];
		rotation.y = model["angles"][1];
		rotation.z = model["angles"][2];
		rotations[i] = rotation;
		i++;
	}
	Engine* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	lastX = config.screenWidth / 2.0f;
	lastY = config.screenHeight / 2.0f;
	std::vector<std::string> faces =
	{
		"data/skybox/fluffballday/FluffballDayLeft.hdr",
		"data/skybox/fluffballday/FluffballDayRight.hdr",
		"data/skybox/fluffballday/FluffballDayTop.hdr",
		"data/skybox/fluffballday/FluffballDayBottom.hdr",
		"data/skybox/fluffballday/FluffballDayFront.hdr",
		"data/skybox/fluffballday/FluffballDayBack.hdr"
	};
	skybox.Init(faces);
}

void SceneDrawingProgram::Draw()
{
	rmt_ScopedOpenGLSample(DrawScene);
	rmt_ScopedCPUSample(DrawSceneCPU, 0);
	glEnable(GL_DEPTH_TEST);
	Engine* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();

	ProcessInput();

	modelShader.Bind();
	projection = glm::perspective(glm::radians(camera.Zoom), (float)config.screenWidth / (float)config.screenHeight, 0.1f, 100.0f);
	modelShader.SetMat4("projection", projection);
	modelShader.SetMat4("view", camera.GetViewMatrix());
	for(auto i = 0l; i < modelNmb;i++)
	{
		glm::mat4 modelMatrix(1.0f);
		modelMatrix = glm::translate(modelMatrix, positions[i]);
		modelMatrix = glm::scale(modelMatrix, scales[i]);
		auto quaternion = glm::quat(rotations[i]);
		modelMatrix = glm::mat4_cast(quaternion)*modelMatrix;
		modelShader.SetMat4("model", modelMatrix);
		models[i]->Draw(modelShader);
	}
	auto view = camera.GetViewMatrix();
	skybox.SetViewMatrix(view);
	skybox.SetProjectionMatrix(projection);
	skybox.Draw();
}

void SceneDrawingProgram::Destroy()
{
}

void SceneDrawingProgram::UpdateUi()
{
	ImGui::Separator();
	float cameraPosition[3];
	cameraPosition[0] = camera.Position.x;
	cameraPosition[1] = camera.Position.y;
	cameraPosition[2] = camera.Position.z;
	ImGui::InputFloat3("Camera Position", cameraPosition);
}

void SceneDrawingProgram::ProcessInput()
{
	Engine* engine = Engine::GetPtr();
	auto& inputManager = engine->GetInputManager();
	auto& camera = engine->GetCamera();
	float dt = engine->GetDeltaTime();
	float cameraSpeed = 1.0f;

#ifdef USE_SDL2
	if (inputManager.GetButton(SDLK_w))
	{
		camera.ProcessKeyboard(FORWARD, engine->GetDeltaTime());
	}
	if (inputManager.GetButton(SDLK_s))
	{
		camera.ProcessKeyboard(BACKWARD, engine->GetDeltaTime());
	}
	if (inputManager.GetButton(SDLK_a))
	{
		camera.ProcessKeyboard(LEFT, engine->GetDeltaTime());
	}
	if (inputManager.GetButton(SDLK_d))
	{
		camera.ProcessKeyboard(RIGHT, engine->GetDeltaTime());
	}
#endif

	auto mousePos = inputManager.GetMousePosition();

	camera.ProcessMouseMovement(mousePos.x, mousePos.y, true);

	camera.ProcessMouseScroll(inputManager.GetMouseWheelDelta());
}


int main(int argc, char** argv)
{
	Engine engine;
	auto& config = engine.GetConfiguration();
	config.screenWidth = 1024;
	config.screenHeight = 1024;
	config.windowName = "Hello Water";
	engine.AddDrawingProgram(new SceneDrawingProgram());
	engine.AddDrawingProgram(new HelloWaterDrawingProgram());

	engine.Init();
	engine.GameLoop();

	return EXIT_SUCCESS;
}

