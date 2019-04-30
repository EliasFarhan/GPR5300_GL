#include <engine.h>
#include <graphics.h>

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"


class HelloLightDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
	void UpdateUi() override;
private:
	Shader objShaderProgram;
	Shader lampShaderProgram;

	//pos + normal
	float vertices[6*36] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	unsigned int VBO = 0, cubeVAO = 0;
	unsigned int lightVAO;

	glm::vec3 lightPos = { 2.0f, 0.0f, 2.0f };

	float ambientStrength = 0.2f;
	float specularStrength = 0.5f;
	float diffuseStrength = 1.0f;
	int specularPow = 256;
	float objectColor[3] = { 1.0f, 0.5f, 0.31f };
	float lightColor[3] = {1.0f,1.0f,1.0f};
};

void HelloLightDrawingProgram::Init()
{
	programName = "HelloLight";

    objShaderProgram.CompileSource(
            "shaders/06_hello_phong_light/light.vert",
            "shaders/06_hello_phong_light/light.frag");
    lampShaderProgram.CompileSource(
            "shaders/06_hello_phong_light/lamp.vert",
            "shaders/06_hello_phong_light/lamp.frag");
	shaders.push_back(&objShaderProgram);
	shaders.push_back(&lampShaderProgram);

	// first, configure the cube's VAO (and VBO)
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	//reusing the cube data, without the normals
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void HelloLightDrawingProgram::Draw()
{
	ProcessInput();

	Engine* engine = Engine::GetPtr();
	Camera& camera = engine->GetCamera();
	auto& config = engine->GetConfiguration();

	lightPos = glm::vec3(2.0f*sin(2.0f*M_PI / 3.0f*engine->GetTimeSinceInit()), lightPos.y, 2.0f*cos(2.0f*M_PI / 3.0f*engine->GetTimeSinceInit()));

	glEnable(GL_DEPTH_TEST);

	objShaderProgram.Bind();
	objShaderProgram.SetFloat("ambientStrength", ambientStrength);
	glUniform1f(glGetUniformLocation(objShaderProgram.GetProgram(), "specularStrength"), specularStrength);
	objShaderProgram.SetFloat("diffuseStrength", diffuseStrength);
	glUniform3fv(glGetUniformLocation(objShaderProgram.GetProgram(), "objectColor"), 1, objectColor);
	glUniform3fv(glGetUniformLocation(objShaderProgram.GetProgram(), "lightColor"), 1, lightColor);
	glUniform1i(glGetUniformLocation(objShaderProgram.GetProgram(), "specularPow"), specularPow);
	glUniform3fv(glGetUniformLocation(objShaderProgram.GetProgram(), "lightPos"), 1, &lightPos[0]);
	glUniform3fv(glGetUniformLocation(objShaderProgram.GetProgram(), "viewPos"), 1, &camera.Position[0]);
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)config.screenWidth / (float)config.screenHeight, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(objShaderProgram.GetProgram(), "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(objShaderProgram.GetProgram(), "view"), 1, GL_FALSE, glm::value_ptr(view));

	// world transformation
	glm::mat4 model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(objShaderProgram.GetProgram(), "model"), 1, GL_FALSE, glm::value_ptr(model));

	// render the cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	// also draw the lamp object
	lampShaderProgram.Bind();
	lampShaderProgram.SetVec3("lightColor", lightColor[0], lightColor[1], lightColor[2]);
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

void HelloLightDrawingProgram::Destroy()
{
}



void HelloLightDrawingProgram::UpdateUi()
{
	ImGui::Separator();
	ImGui::InputFloat("ambientStrength", &ambientStrength);
	ImGui::InputFloat("diffuseStrength", &diffuseStrength);
	ImGui::InputFloat("specularStrength", &specularStrength);
	ImGui::InputInt("specularPow", &specularPow);
	ImGui::ColorEdit3("objectColor", objectColor);
	ImGui::ColorEdit3("lightColor", lightColor);
}


int main(int argc, char** argv)
{
	Engine engine;
	auto& config = engine.GetConfiguration();
	config.screenWidth = 1024;
	config.screenHeight = 1024;
	config.windowName = "Hello Light";
	
	engine.AddDrawingProgram(new HelloLightDrawingProgram());

	engine.Init();
	engine.GameLoop();

	return EXIT_SUCCESS;
}
