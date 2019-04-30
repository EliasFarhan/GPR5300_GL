#pragma once

#include <engine.h>
#include <graphics.h>
#include <glm/glm.hpp>
class Light
{
public:
	virtual ~Light() = default;
	float intensity = 0.5f;
	virtual void Bind(Shader& shader, int index = 0) = 0;
	bool enable = false;
	glm::vec3 position;
};

class DirectionLight : public Light
{
public:
	void Bind(Shader& shader, int index) override;
	glm::vec3 direction;
};

class PointLight : public Light
{
public:
	void Bind(Shader& shader, int index) override;
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;
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