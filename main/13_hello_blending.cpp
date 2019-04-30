#include <map>
#include <engine.h>
#include <graphics.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TRANSPARENT
#define BLENDING
//#define ORDER

class HelloBlendingDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;


private:
	Shader windowShaderProgram;
	Shader grassShaderProgram;

	unsigned VAO;
	unsigned grassTexture;
	unsigned windowTexture;

	unsigned VBO[2];
	unsigned EBO;
	float vertices[12] = {
			0.5f,  0.5f, 0.0f,  // top right
			0.5f, -0.5f, 0.0f,  // bottom right
			-0.5f, -0.5f, 0.0f,  // bottom left
			-0.5f,  0.5f, 0.0f   // top left
	};
	float texCoords[8] = {
			1.0f, 0.0f,	  // top right
			1.0f, 1.0f,   // bottom right
			0.0f, 1.0f,   // bottom left
			0.0f, 0.0f,   // top left
	};
	unsigned int indices[6] = {
		// note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	const static int grassNmb = 3;
	glm::vec3 grassPositions [grassNmb] =
	{
		glm::vec3(1., 0.0,-3.0),
		glm::vec3(1., 0.0,-1.0),
		glm::vec3(-1.,0.0,-2.0),
	};

	const static int windowNmb = 3;

	glm::vec3 windowPositions [windowNmb] =
	{
		glm::vec3(-1., 0.0,-3.0),
		glm::vec3(-1., 0.0,-3.0),
		glm::vec3(1., 0.0, -2.0),
	};

	float windowAngles [windowNmb] =
	{
		glm::radians(45.0f),
		glm::radians(-45.0f),
		0.0f
	};

};

void HelloBlendingDrawingProgram::Init()
{
	programName = "HelloBlending";

	shaders.push_back(&grassShaderProgram);
	shaders.push_back(&windowShaderProgram);
#ifdef BLENDING
    windowShaderProgram.CompileSource(
            "shaders/13_hello_blending/quad.vert",
            "shaders/13_hello_blending/full_transparent.frag");
#else

#ifdef TRANSPARENT
	windowShaderProgram.Init(
		"shaders/13_hello_blending/quad.vert",
		"shaders/13_hello_blending/full_transparent.frag");
#else
	windowShaderProgram.Init(
		"shaders/13_hello_blending/quad.vert",
		"shaders/13_hello_blending/quad.frag");
#endif

#endif

#ifdef TRANSPARENT
    grassShaderProgram.CompileSource(
            "shaders/13_hello_blending/quad.vert",
            "shaders/13_hello_blending/full_transparent.frag");
#else
	grassShaderProgram.Init(
		"shaders/13_hello_blending/quad.vert",
		"shaders/13_hello_blending/quad.frag");
#endif

	grassTexture = stbCreateTexture("data/sprites/grass.png", true, true, true);
	windowTexture = stbCreateTexture("data/sprites/blending_transparent_window.png");

	glGenBuffers(2, &VBO[0]);
	glGenBuffers(1, &EBO);


	glGenVertexArrays(1, &VAO); //like: new VAO()
	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);//Now use our VAO
	//bind vertices data
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//bind texture coords data
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	//bind vertices index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//unbind Vertex Array
	glBindVertexArray(0);

}

void HelloBlendingDrawingProgram::Draw()
{

	ProcessInput();

	glEnable(GL_DEPTH_TEST);
	Engine* engine = Engine::GetPtr();
	Configuration& config = engine->GetConfiguration();
	auto& camera = engine->GetCamera();

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)config.screenWidth / config.screenHeight, 0.1f, 100.0f);

	//Draw grass
	grassShaderProgram.Bind();
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	glBindVertexArray(VAO);

	grassShaderProgram.SetMat4("view", view);
	grassShaderProgram.SetMat4("projection", projection);

	glm::mat4 model = glm::mat4(1.0f); //model transform matrix

	for (auto grassPosition : grassPositions)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, grassPosition);
		grassShaderProgram.SetMat4("model", model);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	//Draw window
#ifdef BLENDING
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
	windowShaderProgram.Bind();
	glBindTexture(GL_TEXTURE_2D, windowTexture);

	windowShaderProgram.SetMat4("view", view);
	windowShaderProgram.SetMat4("projection", projection);
#ifdef ORDER
	std::map<float, glm::vec3> sorted;
	for (auto windowPosition : windowPositions)
	{
		float distance = glm::length(camera.Position - windowPosition);
		sorted[distance] = windowPosition;
	}
	for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
	{
		auto windowPosition = it->second;
#else
	for (int i = 0; i < windowNmb;i++)
	{
		auto windowPosition = windowPositions[i];
#endif
		model = glm::mat4(1.0f);
		model = glm::translate(model, windowPosition);
		model = glm::rotate(model, windowAngles[i], glm::vec3(0,1,0));
		windowShaderProgram.SetMat4("model", model);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
}

void HelloBlendingDrawingProgram::Destroy()
{
	glDeleteVertexArrays(1, &VAO);

	glDeleteBuffers(2, &VBO[0]);
	glDeleteBuffers(2, &EBO);
}



int main(int argc, char** argv)
{
	Engine engine;

	auto& config = engine.GetConfiguration();
	config.screenWidth = 1024;
	config.screenHeight = 1024;
	config.windowName = "Hello Blending";
	config.bgColor = { 1,1,1 };

	engine.AddDrawingProgram(new HelloBlendingDrawingProgram());

	engine.Init();

	engine.GameLoop();
	return EXIT_SUCCESS;
}
