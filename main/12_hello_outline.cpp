#include <engine.h>
#include <graphics.h>
#include <model.h>

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"


class HelloOutlineDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
	void UpdateUi() override;

private:
	Shader modelShaderProgram;
	Shader outlineShaderProgram;

	unsigned int VBO = 0, cubeVAO = 0;

	Model model;

	float outlineMoveScale = 0.1f;
	float outlineColor[3] = { 1.0f,1.0f,1.0f };
};

void HelloOutlineDrawingProgram::Init()
{
	programName = "Hello Outline";

    modelShaderProgram.CompileSource(
            "shaders/12_hello_outline/model.vert",
            "shaders/12_hello_outline/model.frag");
    outlineShaderProgram.CompileSource(
            "shaders/12_hello_outline/outline.vert",
            "shaders/12_hello_outline/outline.frag");
	shaders.push_back(&modelShaderProgram);
	shaders.push_back(&outlineShaderProgram);
	// "data/models/nanosuit/scene.fbx"
	// "data/models/van_gogh_room/Enter a title.obj"
	// 
	model.Init("data/models/nanosuit2/nanosuit.obj");
}

void HelloOutlineDrawingProgram::Draw()
{
	ProcessInput();

	Engine* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	auto& camera = engine->GetCamera();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)config.screenWidth / (float)config.screenHeight, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();

	glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments should update the stencil buffer
	glStencilMask(0xFF); // enable writing to the stencil buffer
	glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)
	modelShaderProgram.Bind();

	modelShaderProgram.SetMat4("view", view);
	modelShaderProgram.SetMat4("projection", projection);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down


	modelShaderProgram.SetMat4("model", model);
	this->model.Draw(modelShaderProgram);
	
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00); // disable writing to the stencil buffer
	glDisable(GL_DEPTH_TEST);

	outlineShaderProgram.Bind();
	outlineShaderProgram.SetMat4("view", view);
	outlineShaderProgram.SetMat4("projection", projection);
	outlineShaderProgram.SetVec3("outlineColor", outlineColor);

	glm::vec3 sides [8]=
	{
		glm::vec3(1.0,0.0,0.0),
		glm::vec3(1.0,1.0,0.0),
		glm::vec3(0.0,1.0,0.0),
		glm::vec3(-1.0,1.0,0.0),
		glm::vec3(-1.0,0.0,0.0),
		glm::vec3(-1.0,-1.0,0.0),
		glm::vec3(0.0,-1.0,0.0),
		glm::vec3(1.0,-1.0,0.0),
	};
	for (const auto side : sides)
	{
		const auto newModel = glm::translate(model, side*outlineMoveScale);
		outlineShaderProgram.SetMat4("model", newModel);
		this->model.Draw(outlineShaderProgram);
	}
	glStencilMask(0xFF);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

}

void HelloOutlineDrawingProgram::Destroy()
{
}

void HelloOutlineDrawingProgram::UpdateUi()
{
	ImGui::Separator();
	ImGui::InputFloat("moveScale", &outlineMoveScale);
	ImGui::ColorEdit3("outlineColor", outlineColor);
}



int main(int argc, char** argv)
{
	Engine engine;
	auto& config = engine.GetConfiguration();
	config.screenWidth = 1024;
	config.screenHeight = 1024;
	config.windowName = "Hello Outline";
	engine.AddDrawingProgram(new HelloOutlineDrawingProgram());

	engine.Init();
	engine.GameLoop();

	return EXIT_SUCCESS;
}