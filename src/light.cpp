#include <light.h>

void DirectionLight::Bind(Shader& shader, int index)
{
	shader.SetVec3("directionLight.direction", direction);
	shader.SetFloat("directionLight.intensity", intensity);
	shader.SetVec3("directionLight.color", color);
}

void PointLight::Bind(Shader& shader, int index)
{
	std::string i = std::to_string(index);

	shader.SetVec3("pointLights[" + i + "].position", position);
	shader.SetFloat("pointLights[" + i + "].distance", distance);
	shader.SetFloat("pointLights[" + i + "].intensity", intensity);
	shader.SetVec3("pointLights[" + i + "].color", color);
}

void SpotLight::Bind(Shader& shader, int index)
{
	std::string i = std::to_string(index);
	shader.SetVec3("spotLights[" + i + "].position", position);
	shader.SetVec3("spotLights[" + i + "].direction", direction);
	shader.SetVec3("spotLights[" + i + "].color", color);
	shader.SetFloat("spotLights[" + i + "].cutOff", glm::cos(glm::radians(cutOff)));
	shader.SetFloat("spotLights[" + i + "].outerCutOff", glm::cos(glm::radians(outerCutOff)));
	shader.SetFloat("spotLights[" + i + "].intensity", intensity);
}
