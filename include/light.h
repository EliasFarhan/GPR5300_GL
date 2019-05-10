#pragma once

#include <engine.h>
#include <graphics.h>
#include <glm/glm.hpp>

class Light
{
public:
	Light() = default;
	virtual ~Light() = default;

	virtual void Bind(Shader& shader, int index = 0) = 0;
	
	float intensity = 0.5f;
	glm::vec3 position;
	glm::vec3 color = glm::vec3(1,1,1);
	bool enable = false;
};

class DirectionLight : public Light
{
public:
	void Bind(Shader& shader, int index) override;
	glm::vec3 direction = glm::vec3(1,0,0);
};

class PointLight : public Light
{
public:
	void Bind(Shader& shader, int index) override;
	float distance = 1.0f;
};

class SpotLight : public Light
{
public:
	void Bind(Shader& shader, int index) override;
	glm::vec3  direction;
	float cutOff = glm::cos(glm::radians(12.5f));
	float outerCutOff = glm::cos(glm::radians(15.0f));
};