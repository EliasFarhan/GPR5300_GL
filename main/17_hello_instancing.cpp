#include <engine.h>
#include <graphics.h>
#include <model.h>
#include <camera.h>
#include "math_utility.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <Remotery.h>

#define INSTANCING
#define MOVEMENT
//#define OPTI_BANDWIDTH
class HelloInstancingDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;

	void CalculateForce();
	void CalculateVelocity();
private:
	Model asteroidModel{};
	Shader modelShader;
	const float asteroidScale = 0.01f;
	size_t planetNmb = 100'000;
	std::vector<glm::vec3> positions;
	std::vector<glm::mat4> modelMatrices;
	std::vector<glm::vec3> velocities;
	std::vector<glm::vec3> forces;
#ifdef INSTANCING
	unsigned int instanceVBO;
#endif

	const float gravityConst = 1000.0f;
	const float centerMass = 1000.0f;
	const float planetMass = 1.0f;
};

void HelloInstancingDrawingProgram::Init()
{
	auto engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	camera.Position = glm::vec3(500.0f, 100.0f, 500.0f);
	programName = "Hello Instancing";

	asteroidModel.Init("data/models/rocks01/rock_01.obj");
	modelShader.CompileSource(
#ifdef INSTANCING
#ifdef OPTI_BANDWIDTH
		"shaders/17_hello_instancing/model_instancing_opti.vert",
#else
		"shaders/17_hello_instancing/model_instancing.vert",
#endif
		"shaders/17_hello_instancing/model_instancing.frag"
#else
		"shaders/engine/model.vert",
		"shaders/engine/model.frag"
#endif
	);
	shaders.push_back(&modelShader);

	positions.resize(planetNmb);
	velocities.resize(planetNmb);
	forces.resize(planetNmb);
	modelMatrices.resize(planetNmb);

	for(auto i = 0u; i < planetNmb;i++)
	{
		const float radius = RandomRange(100, 300);
		const float angle = RandomRange(0, 359);
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 1.0f);
		positions[i] = glm::rotate(position, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f) );
		positions[i] *= radius;

#ifdef INSTANCING
		glm::mat4& model = modelMatrices[i];
		model = glm::mat4(1.0f);
		model = glm::translate(model, positions[i]);
		model = glm::scale(model,
			glm::vec3(asteroidScale, asteroidScale, asteroidScale));
#endif

	}
#ifdef INSTANCING

	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER,
#ifdef OPTI_BANDWIDTH
		planetNmb * sizeof(glm::vec3),
		&positions[0],
#else
		planetNmb * sizeof(glm::mat4),
		&modelMatrices[0],
#endif 
#ifdef MOVEMENT
		GL_DYNAMIC_DRAW
#else
		GL_STATIC_DRAW
#endif
	);
#ifdef OPTI_BANDWIDTH
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
#endif

	unsigned int VAO = asteroidModel.meshes[0].GetVAO();

	glBindVertexArray(VAO);
	GLsizei vec4Size = sizeof(glm::vec4);
#ifdef OPTI_BANDWIDTH
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glVertexAttribDivisor(3, 1);
#else
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
#endif
	glBindVertexArray(0);

#endif
}

void HelloInstancingDrawingProgram::Draw()
{
	rmt_ScopedOpenGLSample(DrawAsteroid);
	rmt_ScopedCPUSample(DrawAsteroidCPU, 0);
	glEnable(GL_DEPTH_TEST);
	ProcessInput();
	Engine* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();

	glm::mat4 projection = glm::perspective(
		glm::radians(camera.Zoom),
		(float)config.screenWidth / (float)config.screenHeight,
		0.1f,
		1000.0f);
	glm::mat4 view = camera.GetViewMatrix();
	
	modelShader.Bind();
	modelShader.SetMat4("projection", projection);
	modelShader.SetMat4("view", view);
#ifdef INSTANCING
	Mesh& asteroidMesh = asteroidModel.meshes[0];
	for (unsigned int i = 0; i < asteroidMesh.textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = asteroidMesh.textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(1);
		else if (name == "texture_specular")
			number = std::to_string(1);

		modelShader.SetInt(("material." + name + number).c_str(), i);
		glBindTexture(GL_TEXTURE_2D, asteroidMesh.textures[i].id);
}
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(asteroidMesh.GetVAO());
	
#ifdef MOVEMENT
	CalculateForce();
	CalculateVelocity();
	for (auto i = 0u; i < planetNmb; i++)
	{
		positions[i] += velocities[i] * engine->GetDeltaTime();
		glm::mat4& model = modelMatrices[i];
		model = glm::mat4(1.0f);
		model = glm::translate(model, positions[i]);
		model = glm::scale(model,
			glm::vec3(asteroidScale, asteroidScale, asteroidScale));
	}
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, 
#ifdef OPTI_BANDWIDTH
		planetNmb * sizeof(glm::vec3),
		&positions[0],
#else
		planetNmb * sizeof(glm::mat4),
		&modelMatrices[0],
#endif 
		GL_DYNAMIC_DRAW);
#endif
	modelShader.SetFloat("asteroidScale", asteroidScale);
	glDrawElementsInstanced(GL_TRIANGLES, asteroidMesh.indices.size(), GL_UNSIGNED_INT, 0, planetNmb);
#else
	CalculateForce();
	CalculateVelocity();
	for (auto i = 0u; i < planetNmb; i++)
	{
#ifdef MOVEMENT
		positions[i] += velocities[i] * engine->GetDeltaTime();
#endif
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, positions[i]);
		model = glm::scale(model,
			glm::vec3(asteroidScale, asteroidScale, asteroidScale));

		modelShader.SetMat4("model", model);
		asteroidModel.Draw(modelShader);
	}
#endif

}

void HelloInstancingDrawingProgram::Destroy()
{
}

void HelloInstancingDrawingProgram::CalculateForce()
{
	rmt_ScopedCPUSample(CalculateForceCPU, 0);
	for (int i = 0; i < planetNmb; i++)
	{
		const auto deltaToCenter = - positions[i];
		const auto r = glm::length(deltaToCenter);
		const auto force = gravityConst * centerMass * planetMass / (r*r);
		forces[i] = deltaToCenter / r * force;  
	}
}

void HelloInstancingDrawingProgram::CalculateVelocity()
{
	rmt_ScopedCPUSample(CalculateVelocityCPU, 0);
	for (int i = 0; i < planetNmb; i++)
	{
		const auto deltaToCenter = -positions[i];
		auto velDir = glm::vec3(-deltaToCenter.z, 0.0f, deltaToCenter.x);
		velDir = glm::normalize(velDir);

		const auto speed = sqrtf(glm::length(forces[i]) / planetMass * glm::length(deltaToCenter));
		velocities[i] = velDir*speed;
	}

}

int main(int argc, char** argv)
{
	Engine engine;

	auto& config = engine.GetConfiguration();
	config.screenWidth = 1024;
	config.screenHeight = 1024;
	config.windowName = "Hello Instancing";
	config.bgColor = { 1,1,1,1 };

	engine.AddDrawingProgram(new HelloInstancingDrawingProgram());

	srand(0);
	engine.Init();
	engine.GameLoop();

	return EXIT_SUCCESS;
}
