#include <engine.h>
#include "graphics.h"
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

class HelloLightCastersDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
private:

	static const int cubeLength = 10;
	Shader objShaderProgram;
	Shader lampShaderProgram;
	//pos + normal
	float vertices[8 * 36] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	glm::vec3 cubePositions[cubeLength] = {
	  glm::vec3(0.0f,  0.0f,  0.0f),
	  glm::vec3(2.0f,  5.0f, -15.0f),
	  glm::vec3(-1.5f, -2.2f, -2.5f),
	  glm::vec3(-3.8f, -2.0f, -12.3f),
	  glm::vec3(2.4f, -0.4f, -3.5f),
	  glm::vec3(-1.7f,  3.0f, -7.5f),
	  glm::vec3(1.3f, -2.0f, -2.5f),
	  glm::vec3(1.5f,  2.0f, -2.5f),
	  glm::vec3(1.5f,  0.2f, -1.5f),
	  glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	float cubeInitialAngle[cubeLength] =
	{
		M_PI / 2.0f,
		M_PI / 4.0f,
		-M_PI / 4.0f,
		-M_PI / 2.0f,
		M_PI * 3.0f / 2.0f,
		M_PI * 3.0f / 4.0f,
		M_PI * 3.0f / 4.0f,
		M_PI / 3.0f,
		M_PI / 6.0f,
		-M_PI / 6.0f
	};

	unsigned int VBO = 0, cubeVAO = 0;
	unsigned int lightVAO;

	glm::vec3 lightPos = { 2.0f, 0.0f, 2.0f };

	int diffuseMapTexture = 0;
	int specularMapTexture = 0;
};

void HelloLightCastersDrawingProgram::Init()
{
	programName = "HelloLightMaps";


    objShaderProgram.CompileSource(
            "shaders/08_hello_lighting_maps/material.vert",
            "shaders/08_hello_lighting_maps/material.frag");
    lampShaderProgram.CompileSource(
            "shaders/08_hello_lighting_maps/lamp.vert",
            "shaders/08_hello_lighting_maps/lamp.frag");
	shaders.push_back(&objShaderProgram);
	shaders.push_back(&lampShaderProgram);

	diffuseMapTexture = stbCreateTexture("data/sprites/container2.png");
	specularMapTexture = stbCreateTexture("data/sprites/container2_specular.png");

	// first, configure the cube's VAO (and VBO)
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture coords attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	//reusing the cube data, without the normals
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void HelloLightCastersDrawingProgram::Draw()
{
	ProcessInput();

	Engine* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	auto& camera = engine->GetCamera();

	lightPos = glm::vec3(5.0f*sin(2.0f*M_PI / 3.0f*engine->GetTimeSinceInit()), lightPos.y, 2.0f*cos(2.0f*M_PI / 3.0f*engine->GetTimeSinceInit()));

	glEnable(GL_DEPTH_TEST);

	objShaderProgram.Bind();
	glUniform3f(glGetUniformLocation(objShaderProgram.GetProgram(), "objectColor"), 1.0f, 0.5f, 0.31f);
	glUniform3fv(glGetUniformLocation(objShaderProgram.GetProgram(), "viewPos"), 1, &camera.Position[0]);
	//material parameters
	glUniform1i(glGetUniformLocation(objShaderProgram.GetProgram(), "material.diffuse"), 0);//TEXTURE0
	glUniform1i(glGetUniformLocation(objShaderProgram.GetProgram(), "material.specular"), 1);//TEXTURE1
	glUniform1f(glGetUniformLocation(objShaderProgram.GetProgram(), "material.shininess"), 64.0f);
	//light parameter
	glUniform3fv(glGetUniformLocation(objShaderProgram.GetProgram(), "light.position"), 1, &lightPos[0]);
	glUniform3f(glGetUniformLocation(objShaderProgram.GetProgram(), "light.ambient"), 0.2f, 0.2f, 0.2f);
	glUniform3f(glGetUniformLocation(objShaderProgram.GetProgram(), "light.diffuse"), 0.5f, 0.5f, 0.5f); // darken the light a bit to fit the scene
	glUniform3f(glGetUniformLocation(objShaderProgram.GetProgram(), "light.specular"), 1.0f, 1.0f, 1.0f);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)config.screenWidth / (float)config.screenHeight, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(objShaderProgram.GetProgram(), "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(objShaderProgram.GetProgram(), "view"), 1, GL_FALSE, glm::value_ptr(view));


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMapTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularMapTexture);
	glm::mat4 model = glm::mat4(1.0f);
	// world transformation
	for (int i = 0; i < cubeLength; i++)
	{
		model = glm::mat4(1.0f);
		model = glm::rotate(model, cubeInitialAngle[i], glm::vec3(0.5f, 1.0f, 0.0f));

		model = glm::translate(model, cubePositions[i]);

		glUniformMatrix4fv(glGetUniformLocation(objShaderProgram.GetProgram(), "model"), 1, GL_FALSE, glm::value_ptr(model));

		// render the cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
	// also draw the lamp object
	lampShaderProgram.Bind();
	glUniformMatrix4fv(glGetUniformLocation(lampShaderProgram.GetProgram(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(lampShaderProgram.GetProgram(), "view"), 1, GL_FALSE, glm::value_ptr(view));

	model = glm::mat4(1.0f);
	model = glm::translate(model, lightPos);
	model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
	glUniformMatrix4fv(glGetUniformLocation(lampShaderProgram.GetProgram(), "model"), 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(lightVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void HelloLightCastersDrawingProgram::Destroy()
{
}


int main(int argc, char** argv)
{
	Engine engine;

	auto& config = engine.GetConfiguration();
	config.screenWidth = 1024;
	config.screenHeight = 1024;
	config.windowName = "Hello Light Maps";

	engine.AddDrawingProgram(new HelloLightCastersDrawingProgram());

	engine.Init();
	engine.GameLoop();

	return EXIT_SUCCESS;
}
