#include <map>
#include <vector>

#include <engine.h>
#include <graphics.h>
#include <camera.h>
#include <model.h>
#include <geometry.h>

#include <Remotery.h>
#include "file_utility.h"

#include <json.hpp>
using json = nlohmann::json;
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>

#include "imgui.h"

class SceneDrawingProgram;

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
	float waterPosition[3]{0.0f,2.0f,0.0f};

	Shader reflectionModelShader;
	unsigned int reflectionColorBuffer;
	unsigned int reflectionBuffer;
	unsigned int reflectionDepthBuffer;
	Shader frameBufferShaderProgram;
	Plane plane;

	Shader refractionModelShader;
	unsigned int refractionColorBuffer;
	unsigned int refractionDepthBuffer;
	unsigned int refractionBuffer;
	unsigned int refractionStencilBuffer;
	unsigned dudvMap;
	unsigned normalMap;
};

class SceneDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
	void UpdateUi() override;
	//Getters for Water Drawing Program
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
	sceneCamera = &engine->GetCamera();
	sceneCamera->Position = glm::vec3(0.0f, 3.0f, 10.0f);
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

	//screen quad
	plane.Init();
	dudvMap = stbCreateTexture("data/sprites/waveDUDV.png");
	normalMap = stbCreateTexture("data/sprites/waveNM.png");
	reflectionModelShader.CompileSource(
		"shaders/98_hello_water/reflection.vert", 
		"shaders/98_hello_water/reflection.frag");
	shaders.push_back(&reflectionModelShader);
	
	//Generate under water refelction map framebuffer
	glGenFramebuffers(1, &reflectionBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, reflectionBuffer);

	glGenTextures(1, &reflectionColorBuffer);
	glBindTexture(GL_TEXTURE_2D, reflectionColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, config.screenWidth, config.screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionColorBuffer, 0);

	glGenRenderbuffers(1, &reflectionDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, reflectionDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, config.screenWidth, config.screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, reflectionDepthBuffer);
	frameBufferShaderProgram.CompileSource(
		"shaders/98_hello_water/blending.vert",
		"shaders/98_hello_water/blending.frag"
	);
	
	//Generate under water refelction map framebuffer
	refractionModelShader.CompileSource(
		"shaders/98_hello_water/refraction.vert",
		"shaders/98_hello_water/refraction.frag");
	shaders.push_back(&refractionModelShader);
	glGenFramebuffers(1, &refractionBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, refractionBuffer);

	glGenTextures(1, &refractionColorBuffer);
	glBindTexture(GL_TEXTURE_2D, refractionColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, config.screenWidth, config.screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractionColorBuffer, 0);

	glGenTextures(1, &refractionDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, refractionDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		config.screenWidth, config.screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindFramebuffer(GL_FRAMEBUFFER, refractionBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, refractionDepthBuffer, 0);

	glGenRenderbuffers(1, &refractionStencilBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, refractionStencilBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX1, config.screenWidth, config.screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, refractionDepthBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void HelloWaterDrawingProgram::Draw()
{
	rmt_ScopedOpenGLSample(DrawWater);
	rmt_ScopedCPUSample(DrawWaterCPU, 0);
	Engine* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	Skybox& skybox = scene->GetSkybox();

	glm::mat4 stencilWaterModel = glm::mat4(1.0f);
	//==================REFLECTION==================
	{
		//Invert Camera
		underWaterCamera.Front = glm::reflect(sceneCamera->Front, glm::vec3(0.0f, 1.0f, 0.0f));
		underWaterCamera.Up = glm::reflect(sceneCamera->Up, glm::vec3(0.0f, 1.0f, 0.0f));
		underWaterCamera.Position = sceneCamera->Position;
		underWaterCamera.Position.y = underWaterCamera.Position.y - 2 * abs(underWaterCamera.Position.y - waterPosition[1]);

		//Under Water framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, reflectionBuffer);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Draw scene on top of water in reflection map
		reflectionModelShader.Bind();
		reflectionModelShader.SetFloat("waterHeight", waterPosition[1]);
		reflectionModelShader.SetMat4("projection", scene->GetProjection());
		reflectionModelShader.SetMat4("view", underWaterCamera.GetViewMatrix());
		for (auto i = 0l; i < scene->GetModelNmb(); i++)
		{
			glm::mat4 modelMatrix(1.0f);
			modelMatrix = glm::translate(modelMatrix, scene->GetPositions()[i]);
			modelMatrix = glm::scale(modelMatrix, scene->GetScales()[i]);
			glm::quat quaternion = glm::quat(scene->GetRotations()[i]);
			modelMatrix = glm::mat4_cast(quaternion)*modelMatrix;

			reflectionModelShader.SetMat4("model", modelMatrix);

			scene->GetModels()[i]->Draw(reflectionModelShader);
		}
		
		const auto underWaterView = underWaterCamera.GetViewMatrix();
		skybox.SetViewMatrix(underWaterView);
		skybox.SetProjectionMatrix(scene->GetProjection());
		skybox.Draw();

	}
	//==================REFRACTION==================
	{

		glBindFramebuffer(GL_FRAMEBUFFER, refractionBuffer);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Draw scene on top of water in reflection map
		refractionModelShader.Bind();
		refractionModelShader.SetFloat("waterHeight", waterPosition[1]);
		refractionModelShader.SetMat4("projection", scene->GetProjection());
		refractionModelShader.SetMat4("view", sceneCamera->GetViewMatrix());
		for (auto i = 0l; i < scene->GetModelNmb(); i++)
		{
			glm::mat4 modelMatrix(1.0f);
			modelMatrix = glm::translate(modelMatrix, scene->GetPositions()[i]);
			modelMatrix = glm::scale(modelMatrix, scene->GetScales()[i]);
			glm::quat quaternion = glm::quat(scene->GetRotations()[i]);
			modelMatrix = glm::mat4_cast(quaternion)*modelMatrix;

			refractionModelShader.SetMat4("model", modelMatrix);

			scene->GetModels()[i]->Draw(refractionModelShader);
		}
		skybox.SetViewMatrix(sceneCamera->GetViewMatrix());
		skybox.SetProjectionMatrix(scene->GetProjection());
		skybox.Draw();
	}
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		frameBufferShaderProgram.Bind();
        stencilWaterModel = glm::mat4(1.0f);
        stencilWaterModel = glm::translate(stencilWaterModel, glm::vec3(waterPosition[0], waterPosition[1], waterPosition[2]));

        stencilWaterModel = glm::scale(stencilWaterModel, glm::vec3(5.0f, 5.0f, 5.0f));

		frameBufferShaderProgram.SetMat4("model", stencilWaterModel);
		frameBufferShaderProgram.SetMat4("projection", scene->GetProjection());
		frameBufferShaderProgram.SetMat4("view", sceneCamera->GetViewMatrix());
		frameBufferShaderProgram.SetVec3("viewPos", sceneCamera->Position);
		frameBufferShaderProgram.SetInt("reflectionMap", 0);
		frameBufferShaderProgram.SetInt("refractionMap", 1);
		frameBufferShaderProgram.SetInt("dudvMap", 2);
		frameBufferShaderProgram.SetInt("depthMap", 3);
		frameBufferShaderProgram.SetInt("normalMap", 4);
		frameBufferShaderProgram.SetFloat("timeSinceStart", engine->GetTimeSinceInit());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, reflectionColorBuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, refractionColorBuffer);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, dudvMap);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, refractionDepthBuffer);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, normalMap);

		glBindVertexArray(waterVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
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
		"shaders/engine/model_diffuse.vert", 
		"shaders/engine/model_diffuse.frag");
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
	Engine* engine = Engine::GetPtr();
	Camera& camera = engine->GetCamera();
	rmt_ScopedOpenGLSample(DrawScene);
	rmt_ScopedCPUSample(DrawSceneCPU, 0);
	glEnable(GL_DEPTH_TEST);
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

