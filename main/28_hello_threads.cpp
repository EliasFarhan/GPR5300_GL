#include <engine.h>
#include <graphics.h>
#include <model.h>
#include <camera.h>
#include "math_utility.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>
#include <random>

#include <Remotery.h>

const size_t planetNmb = 100'000;
enum ThreadId
{
	App = 0,
	Culling = 1,
	Render = 2
};
struct Scene
{
	std::vector<glm::vec3> positions = std::vector<glm::vec3>(planetNmb);
	std::vector<glm::vec3> velocities = std::vector<glm::vec3>(planetNmb);
	std::vector <glm::vec3> forces = std::vector<glm::vec3>(planetNmb);
};
struct SceneBuffer
{
	std::vector<glm::vec3> positions = std::vector<glm::vec3>(planetNmb);
	Camera camera;
};

struct CommandBuffer
{
	size_t length = 0;
	Camera camera;
	std::vector<glm::vec3> positions = std::vector<glm::vec3>(planetNmb);
};

struct BoundingSphere
{
	glm::vec3 center = glm::vec3(0.0f);
	float radius = 0.0f;
};

class HelloInstancingDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;

	void CalculateForce();
	void CalculateVelocity();

	void App();//on other thread
	void Culling();//on other thread
private:
	Model asteroidModel{};
	Shader modelShader;
	const float asteroidScale = 0.01f;
	
	unsigned int instanceVBO;

	const float gravityConst = 1000.0f;
	const float centerMass = 1000.0f;
	const float planetMass = 1.0f;

	Scene scene;
	SceneBuffer* sceneBuffers = nullptr;//scene arrays
	CommandBuffer* commandBuffers = nullptr; //command buffers arrays
	std::thread threads[2];
	std::mutex mutex;
	std::condition_variable condVariable;
	std::atomic<bool> isFinish = false;
	std::atomic<bool> threadStarted[3] = {false, false, false};

	std::vector<BoundingSphere> boundingSpheres = std::vector<BoundingSphere>(planetNmb);
	int frameIndex[3] = { 0, 0, 0 };
	int index = 0;
	const float zNear = 0.1f;
	const float zFar = 2000.0f;
};

void HelloInstancingDrawingProgram::Init()
{
	auto engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	camera.Position = glm::vec3(.0f, .0f, .0f);
	programName = "Hello Instancing";

	asteroidModel.Init("data/models/rocks01/rock_01.obj", true);
	modelShader.CompileSource(
		"shaders/17_hello_instancing/model_instancing_opti.vert",
		"shaders/17_hello_instancing/model_instancing.frag"

	);
	shaders.push_back(&modelShader);

	sceneBuffers = new SceneBuffer[2];
	commandBuffers = new CommandBuffer[2];
	std::uniform_real_distribution<float> randomRadius(100.0f,500.0f); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
	std::uniform_real_distribution<float> randomAngle(0.0f, 360.0f); // generates random floats between 0.0 and 1.0

	for (auto i = 0u; i < planetNmb; i++)
	{
		const float radius = randomRadius(generator);
		const float angle = randomAngle(generator);
		const glm::vec3 position = glm::vec3(0.0f, 0.0f, 1.0f);
		scene.positions[i] = glm::rotate(position, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		scene.positions[i] *= radius;
	}
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER,
		planetNmb * sizeof(glm::vec3),
		&sceneBuffers[0].positions[0],
		GL_DYNAMIC_DRAW
	);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);


	unsigned int VAO = asteroidModel.meshes[0].GetVAO();

	glBindVertexArray(VAO);
	GLsizei vec4Size = sizeof(glm::vec4);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glVertexAttribDivisor(3, 1);

	glBindVertexArray(0);

	threads[0] = std::thread(&HelloInstancingDrawingProgram::App, this);
	threads[1] = std::thread(&HelloInstancingDrawingProgram::Culling, this);
	threads[1].detach();
	threads[0].detach();
}

void HelloInstancingDrawingProgram::Draw()
{
	rmt_ScopedOpenGLSample(DrawAsteroidGPU);
	rmt_ScopedCPUSample(DrawAsteroidCPU, 0);
	threadStarted[ThreadId::Render] = true;
	glEnable(GL_DEPTH_TEST);
	ProcessInput();
	Engine* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();


	//Get current command buffer
	const bool multiReady = (threadStarted[ThreadId::Culling] && threadStarted[ThreadId::App]);
	{
		std::unique_lock<std::mutex> lock(mutex);
		frameIndex[ThreadId::App] = index;
		frameIndex[ThreadId::Culling] = index - 1;
		frameIndex[ThreadId::Render] = index - 2;
		
		condVariable.notify_all(); 
	}
	if(frameIndex[ThreadId::Render] >= 0)
	{
		CommandBuffer& commandBuffer = commandBuffers[frameIndex[Render]%2];

		const glm::mat4 projection = glm::perspective(
			glm::radians(commandBuffer.camera.Zoom),
			(float)config.screenWidth / (float)config.screenHeight,
			zNear,
			zFar);
		const glm::mat4 view = commandBuffer.camera.GetViewMatrix();
		modelShader.Bind();
		modelShader.SetMat4("projection", projection);
		modelShader.SetMat4("view", view);

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
			std::string materialParameter = ("material." + name + number);
			modelShader.SetInt(materialParameter, i);
			glBindTexture(GL_TEXTURE_2D, asteroidMesh.textures[i].id);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(asteroidMesh.GetVAO());


		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER,
			commandBuffer.length * sizeof(glm::vec3),
			&(commandBuffer.positions[0]),
			GL_DYNAMIC_DRAW);

		modelShader.SetFloat("asteroidScale", asteroidScale);
		glDrawElementsInstanced(GL_TRIANGLES, asteroidMesh.indices.size(), GL_UNSIGNED_INT, 0, commandBuffer.length);

	}
	if(multiReady)
	{
		index++;
	}
}

void HelloInstancingDrawingProgram::Destroy()
{
	delete[](sceneBuffers);
	delete[](commandBuffers);
}

void HelloInstancingDrawingProgram::CalculateForce()
{
	rmt_ScopedCPUSample(CalculateForceCPU, 0);
	
	for (int i = 0; i < planetNmb; i++)
	{
		const auto deltaToCenter = -scene.positions[i];
		const auto r = glm::length(deltaToCenter);
		const auto force = gravityConst * centerMass * planetMass / (r*r);
		scene.forces[i] = deltaToCenter / r * force;
	}
}

void HelloInstancingDrawingProgram::CalculateVelocity()
{
	rmt_ScopedCPUSample(CalculateVelocityCPU, 0);
	for (int i = 0; i < planetNmb; i++)
	{
		const auto deltaToCenter = -scene.positions[i];
		auto velDir = glm::vec3(-deltaToCenter.z, 0.0f, deltaToCenter.x);
		velDir = glm::normalize(velDir);

		const auto speed = sqrtf(glm::length(scene.forces[i]) / planetMass * glm::length(deltaToCenter));
		scene.velocities[i] = velDir * speed;
	}

}

void HelloInstancingDrawingProgram::App()
{

	Engine* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();


	threadStarted[ThreadId::App] = true;
	while(!isFinish)
	{
		{
			std::unique_lock<std::mutex> lock(mutex);
			condVariable.wait(lock);
			threadStarted[ThreadId::App] = false;
		}
		{
			rmt_ScopedCPUSample(AppCPU, 0);
			
			CalculateForce();
			CalculateVelocity();
			rmt_BeginCPUSample(CopyPositionCPU, 0);
			for (auto i = 0u; i < planetNmb; i++)
			{
				scene.positions[i] += scene.velocities[i] * engine->GetDeltaTime();
			}

			SceneBuffer& sceneBuffer = sceneBuffers[frameIndex[ThreadId::App] % 2];
			memcpy(&sceneBuffer.positions[0], &scene.positions[0], sizeof(glm::vec3) * planetNmb);
			sceneBuffer.camera = camera;
			rmt_EndCPUSample();
			threadStarted[ThreadId::App] = true;
		}
		
	}
}

void HelloInstancingDrawingProgram::Culling()
{
	Engine* engine = Engine::GetPtr();
	auto& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();
	threadStarted[ThreadId::Culling] = true;
	while (!isFinish)
	{
		{		
			std::unique_lock<std::mutex> lock(mutex);
			condVariable.wait(lock);
			threadStarted[ThreadId::Culling] = false;
		}
		if(frameIndex[ThreadId::Culling] >= 0)
		{
			rmt_ScopedCPUSample(CullingCPU, 0);

			const int i = frameIndex[ThreadId::Culling] % 2;
			const SceneBuffer& sceneBuffer = sceneBuffers[i];
			//Generate the list of spheres
			rmt_BeginCPUSample(CalculateSphereCPU, 0);
			for(size_t j = 0; j < planetNmb; j++)
			{

				BoundingSphere& boundingSphere = boundingSpheres[j];
				boundingSphere.center = asteroidModel.modelCenter+sceneBuffer.positions[j];
				boundingSphere.radius = asteroidModel.modelRadius * asteroidScale;
			}
			rmt_EndCPUSample(); //Generate sphere

			//frustum culling

			rmt_BeginCPUSample(FrustumCullingCPU, 0);
			size_t planetCount = 0;

			CommandBuffer& commandBuffer = commandBuffers[i];
			const float aspect = static_cast<float>(config.screenWidth) / static_cast<float>(config.screenHeight);
			const glm::vec3 leftDir = glm::normalize(
				glm::rotate(sceneBuffer.camera.Front, glm::radians(camera.Zoom) / 2.0f * aspect, sceneBuffer.camera.Up));
			const glm::vec3 leftNormal = glm::normalize(glm::cross(leftDir, sceneBuffer.camera.Up));
			
			const glm::vec3 rightDir = glm::normalize(
				glm::rotate(sceneBuffer.camera.Front, -glm::radians(camera.Zoom) / 2.0f * aspect, sceneBuffer.camera.Up));
			const glm::vec3 rightNormal = glm::normalize(-glm::cross(rightDir, sceneBuffer.camera.Up));
			
			const glm::vec3 upDir = glm::normalize(
				glm::rotate(sceneBuffer.camera.Front, -glm::radians(camera.Zoom) / 2.0f, sceneBuffer.camera.Right));
			const glm::vec3 upNormal = glm::normalize(glm::cross(upDir, sceneBuffer.camera.Right));
			
			const glm::vec3 downDir = glm::normalize(
				glm::rotate(sceneBuffer.camera.Front, glm::radians(camera.Zoom) / 2.0f, sceneBuffer.camera.Right));
			const glm::vec3 downNormal = glm::normalize(-glm::cross(downDir, sceneBuffer.camera.Right));
			for(size_t j = 0; j < planetNmb; j++)
			{
				BoundingSphere& boundingSphere = boundingSpheres[j];
				const glm::vec3 cameraToSphere = boundingSphere.center - sceneBuffer.camera.Position;
				//near culling
				if(glm::dot(cameraToSphere, sceneBuffer.camera.Front)  < zNear + boundingSphere.radius)
				{
					continue;
				}
				//far culling
				if (glm::dot(cameraToSphere, sceneBuffer.camera.Front) > zFar - boundingSphere.radius)
				{
					continue;
				}
				//left culling
				if (glm::dot(cameraToSphere, leftNormal) < boundingSphere.radius)
				{
					continue;
				}
				
				//right culling
				if (glm::dot(cameraToSphere, rightNormal) < boundingSphere.radius)
				{
					continue;
				}
				
				//up culling
				if (glm::dot(cameraToSphere, upNormal) > -boundingSphere.radius)
				{
					continue;
				}
				
				//down culling
				if (glm::dot(cameraToSphere, downNormal) > -boundingSphere.radius)
				{
					continue;
				}
				

				commandBuffer.positions[planetCount] = sceneBuffer.positions[j];
				planetCount++;
				
			}
			rmt_EndCPUSample();//Frustum Culling
			commandBuffer.camera = sceneBuffer.camera;
			commandBuffer.length = planetCount;
			threadStarted[ThreadId::Culling] = true;
		}
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
	//config.vsync = true;
	engine.AddDrawingProgram(new HelloInstancingDrawingProgram());

	srand(0);
	engine.Init();
	engine.GameLoop();
	return EXIT_SUCCESS;
}
