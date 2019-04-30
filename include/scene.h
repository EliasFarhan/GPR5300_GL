#pragma once

#include <engine.h>
#include <graphics.h>
#include <camera.h>
#include <glm/glm.hpp>
#include <model.h>
#include <map>
#include "light.h"

class Scene
{
public:
	void Init();
	std::vector<Model*>& GetModels() { return models; }
	std::vector<glm::vec3>& GetPositions() { return positions; }
	std::vector<glm::vec3>& GetScales() { return scales; }
	std::vector<glm::vec3>& GetRotations() { return rotations; }
	void SetScenePath(std::string jsonPath) { this->jsonPath = jsonPath; }
	size_t GetModelNmb() { return modelNmb; }

	void BindLights(Shader& shader);
private:
	std::string jsonPath;
	size_t modelNmb;
	std::vector<Model*> models;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> scales;
	std::vector<glm::vec3> rotations;
	std::map<std::string, Model> modelMap;
	//Lights
	std::vector<PointLight> pointLights;
	std::vector<SpotLight> spotLights;
	DirectionLight directionLight;
	float ambient = 0.0f;
};

class SceneDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
	void ProcessInput();
	Scene& GetScene() { return scene; }
private:
	Scene scene = {};
	//Camera camera = Camera(glm::vec3(0.0f, 3.0f, 10.0f));
	Shader modelShader;
	glm::mat4 projection;
};