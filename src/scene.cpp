#include <scene.h>

#include "file_utility.h"
#include <Remotery.h>
#include <glm/gtc/quaternion.inl>
#include <glm/detail/type_quat.hpp>
#include <json_utility.h>



void Scene::Init()
{
    const auto sceneJsonPtr = LoadJson(jsonPath);
    const auto& sceneJson = *sceneJsonPtr;
	modelNmb = sceneJson["models"].size();
	models.resize(modelNmb);
	positions.resize(modelNmb);
	scales.resize(modelNmb);
	rotations.resize(modelNmb);
	int i = 0;
	for (auto& model : sceneJson["models"])
	{
		const std::string modelName = model["model"];
		if (modelMap.find(modelName) == modelMap.end())
		{
			//Load model
			modelMap[modelName] = Model();
			modelMap[modelName].Init(modelName.c_str());
		}
		models[i] = &modelMap[modelName];
		positions[i] = ConvertVec3FromJson(model["position"]);
		scales[i] = ConvertVec3FromJson(model["scale"]);
		rotations[i] = ConvertVec3FromJson(model["angles"]);
		i++;
	}
	json lightsJson = sceneJson["lights"];
	ambient = lightsJson["ambient"];
	for(auto& pointLightJson : lightsJson["point_lights"])
	{
		PointLight pointLight;
		pointLight.position = ConvertVec3FromJson(pointLightJson["position"]);
		pointLight.intensity = pointLightJson["intensity"];
		pointLight.enable = pointLightJson["enable"];
		pointLight.distance = pointLightJson["distance"];
		pointLights.push_back(pointLight);
	}
	for (auto& spotLightJson : lightsJson["spot_lights"])
	{
		SpotLight spotLight;
		spotLight.position = ConvertVec3FromJson(spotLightJson["position"]);
		spotLight.direction = ConvertVec3FromJson(spotLightJson["direction"]);
		spotLight.cutOff = spotLightJson["cutOff"];
		spotLight.outerCutOff = spotLightJson["outerCutOff"];
		spotLight.intensity = spotLightJson["intensity"];
		spotLights.push_back(spotLight);
	}
	{
		json directionLightJson = lightsJson["direction_light"];
		DirectionLight directionLight;
		directionLight.enable = directionLightJson["enable"];
		if (directionLight.enable)
		{
			directionLight.direction = ConvertVec3FromJson(directionLightJson["direction"]);
			directionLight.intensity = directionLightJson["intensity"];
			directionLight.position = ConvertVec3FromJson(directionLightJson["position"]);
		}
		this->directionLight = directionLight;

	}
}

void Scene::BindLights(Shader& shader)
{
	int i = 0;
	for(auto& pointLight : pointLights)
	{
		if(pointLight.enable)
		{
			pointLight.Bind(shader, i);
			i++;
		}
	}
	shader.SetInt("pointLightsNmb", i);
	i = 0;
	for (auto& spotLight : spotLights)
	{
		if (spotLight.enable)
		{
			spotLight.Bind(shader, i);
			i++;
		}
	}
	shader.SetInt("spotLightsNmb", i);
	shader.SetBool("directionalLightEnable", directionLight.enable);
	if(directionLight.enable)
	{
		directionLight.Bind(shader, 0);
	}
	shader.SetFloat("ambientIntensity", ambient);
}

void SceneDrawingProgram::Init()
{
	programName = "Scene Drawing Program";
	scene.Init();
	modelShader.CompileSource(
		"shaders/engine/model.vert",
		"shaders/engine/model.frag");
	shaders.push_back(&modelShader);
}
void SceneDrawingProgram::Draw()
{
	rmt_ScopedOpenGLSample(DrawScene);
	rmt_ScopedCPUSample(DrawSceneCPU, 0);

	ProcessInput();

	glEnable(GL_DEPTH_TEST);
	Engine* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();
	projection = glm::perspective(
		glm::radians(camera.Zoom), 
		(float)config.screenWidth / (float)config.screenHeight, 
		0.1f, 
		100.0f);

	modelShader.Bind();
	scene.BindLights(modelShader);
	modelShader.SetMat4("projection", projection);
	modelShader.SetMat4("view", camera.GetViewMatrix());
	for (auto i = 0u; i < scene.GetModelNmb(); i++)
	{
		glm::mat4 modelMatrix(1.0f);
		modelMatrix = glm::translate(modelMatrix, scene.GetPositions()[i]);
		modelMatrix = glm::scale(modelMatrix, scene.GetScales()[i]);
		auto quaternion = glm::quat(scene.GetRotations()[i]);
		modelMatrix = glm::mat4_cast(quaternion)*modelMatrix;
		modelShader.SetMat4("model", modelMatrix);
		scene.GetModels()[i]->Draw(modelShader);
	}
}
void SceneDrawingProgram::Destroy()
{
	
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
