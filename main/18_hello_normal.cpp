#include <engine.h>
#include <graphics.h>
#include "geometry.h"

#include <imgui.h>

//#define TANGENT
class HelloNormalDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void ProcessInput();
	void Draw() override;
	void Destroy() override;
	void UpdateUi() override;
private:
	unsigned int diffuseMap;
	unsigned int normalMap;
	Shader normalShader;
	Shader withoutNormalShader;
	Plane plane;
	float lightPos[3] = {0,0,1};
	bool enableNormal = true;
};

void HelloNormalDrawingProgram::Init()
{
	programName = "Normal Map";

	normalShader.CompileSource(
#ifdef TANGENT
		"shaders/18_hello_normal/normal.vert", 
		"shaders/18_hello_normal/normal.frag"
#else
		"shaders/18_hello_normal/bad_normal.vert",
		"shaders/18_hello_normal/bad_normal.frag"
#endif
	);
	shaders.push_back(&normalShader);
	withoutNormalShader.CompileSource(
        "shaders/18_hello_normal/without_normal.vert",
        "shaders/18_hello_normal/without_normal.frag"
    );
    shaders.push_back(&withoutNormalShader);
	plane.Init();

	diffuseMap = stbCreateTexture("data/sprites/brickwall.jpg");
	normalMap = stbCreateTexture("data/sprites/brickwall_normal.jpg");

}

void HelloNormalDrawingProgram::ProcessInput()
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

void HelloNormalDrawingProgram::Draw()
{
	Engine* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	auto& camera = engine->GetCamera();
	ProcessInput();
	glEnable(GL_DEPTH_TEST);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, engine->GetTimeSinceInit()*glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)config.screenWidth / (float)config.screenHeight, 0.1f, 100.0f);

	Shader& currentShader = enableNormal?normalShader:withoutNormalShader;
    currentShader.Bind();
    currentShader.SetMat4("model", model);
    currentShader.SetMat4("view", camera.GetViewMatrix());
    currentShader.SetMat4("projection", projection);

    currentShader.SetVec3("viewPos", camera.Position);
    currentShader.SetVec3("lightPos", lightPos);


    currentShader.SetInt("diffuseMap", 0);
    currentShader.SetInt("normalMap", 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);

	plane.Draw();
}

void HelloNormalDrawingProgram::Destroy()
{
}

void HelloNormalDrawingProgram::UpdateUi()
{
	ImGui::Separator();
	ImGui::InputFloat3("LightPos", lightPos);
	ImGui::Checkbox("Enable Normal Map", &enableNormal);
}

int main(int argc, char** argv)
{
	Engine engine;

	auto& config = engine.GetConfiguration();
	config.screenWidth = 1024;
	config.screenHeight = 1024;
	config.windowName = "Hello Normal";
	config.bgColor = { 1,1,1,1 };
	engine.AddDrawingProgram(new HelloNormalDrawingProgram());

	srand(0);
	engine.Init();
	engine.GameLoop();

	return EXIT_SUCCESS;
}
