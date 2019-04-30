#include <engine.h>
#include <graphics.h>
#include "model.h"
#include "imgui.h"


//#define REFRACTION

#ifdef REFRACTION
float refractiveIndexes []=
{
	1.0f,
	1.33f,
	1.309f,
	1.52f,
	2.42f
};
enum class RefractriveMaterial
{
	Air,
	Water,
	Ice,
	Glass,
	Diamond
};
const char* refractriveMaterialName[] =
{
	"Air",
	"Water",
	"Ice",
	"Glass",
	"Diamond"
};
#endif
class HelloCubemapsDrawingProgram : public DrawingProgram
{
public:
	~HelloCubemapsDrawingProgram() = default;
	void Init() override;
	void Draw() override;
	void Destroy() override;
	void UpdateUi() override;
private:
	unsigned int cubemapTexture;
	Shader cubemapShader;
	GLuint cubeMapVAO;
	
	float vertices[5 * 36] =
	{
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	};

	float cubeVertices[8 * 36] = {
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
	unsigned int cubeVAO, cubeVBO;
	GLuint cubeMapVBO;

	Model model;
	Shader modelShaderProgram;
	float reflectionValue = 0.5f;
	int diffuseMapTexture = 0;
	int specularMapTexture = 0;
#ifdef REFRACTION
	int currentRefractiveMaterial = (int)RefractriveMaterial::Water;
#endif
};

void HelloCubemapsDrawingProgram::Init()
{
	programName = "Hello Cubemaps";

    cubemapShader.CompileSource(
            "shaders/16_hello_cubemaps/cubemaps.vert",
            "shaders/16_hello_cubemaps/cubemaps.frag"
    );
	shaders.push_back(&cubemapShader);
	/*std::vector<std::string> faces =
	{
		"data/skybox/nebula/purplenebula_lf.tga",
		"data/skybox/nebula/purplenebula_rt.tga",
		"data/skybox/nebula/purplenebula_up.tga",
		"data/skybox/nebula/purplenebula_dn.tga",
		"data/skybox/nebula/purplenebula_ft.tga",
		"data/skybox/nebula/purplenebula_bk.tga"
	};*/
	/*std::vector<std::string> faces =
	{
		"data/skybox/emerald/emeraldfog_lf.tga",
		"data/skybox/emerald/emeraldfog_rt.tga",
		"data/skybox/emerald/emeraldfog_up.tga",
		"data/skybox/emerald/emeraldfog_dn.tga",
		"data/skybox/emerald/emeraldfog_ft.tga",
		"data/skybox/emerald/emeraldfog_bk.tga"
	};*/
	std::vector<std::string> faces =
	{
		"data/skybox/fluffballday/FluffballDayLeft.hdr",
		"data/skybox/fluffballday/FluffballDayRight.hdr",
		"data/skybox/fluffballday/FluffballDayTop.hdr",
		"data/skybox/fluffballday/FluffballDayBottom.hdr",
		"data/skybox/fluffballday/FluffballDayFront.hdr",
		"data/skybox/fluffballday/FluffballDayBack.hdr"
	};
	cubemapTexture = LoadCubemap(faces);

	glGenVertexArrays(1, &cubeMapVAO);
	glGenBuffers(1, &cubeMapVBO);

	glBindVertexArray(cubeMapVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeMapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	//cube vao
	// first, configure the cube's VAO (and VBO)
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture coords attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2); 
	glBindVertexArray(0);
	diffuseMapTexture = stbCreateTexture("data/sprites/container2.png");
	specularMapTexture = stbCreateTexture("data/sprites/container2_specular.png");
	

	model.Init("data/models/nanosuit2/nanosuit.obj");
	modelShaderProgram.CompileSource(
		"shaders/16_hello_cubemaps/model_refl.vert",
#ifdef REFRACTION
		"shaders/16_hello_cubemaps/model_refr.frag"
#else
		"shaders/16_hello_cubemaps/model_refl.frag"
#endif
	);
	shaders.push_back(&modelShaderProgram);

	
}


void HelloCubemapsDrawingProgram::Draw()
{
	ProcessInput();
	Engine* engine = Engine::GetPtr();
	auto& config = engine->GetConfiguration();
	auto& camera = engine->GetCamera();

	const glm::mat4 projection = glm::perspective(
		glm::radians(camera.Zoom), 
		(float)config.screenWidth / (float)config.screenHeight, 
		0.1f, 
		100.0f);
	const glm::mat4 view = camera.GetViewMatrix();

	glEnable(GL_DEPTH_TEST);

	//Show model
	modelShaderProgram.Bind();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
	
	modelShaderProgram.SetMat4("model", model);
	modelShaderProgram.SetMat4("projection", projection);
	modelShaderProgram.SetMat4("view", view);

	modelShaderProgram.SetFloat("reflectionValue", reflectionValue);
#ifdef REFRACTION
	modelShaderProgram.SetFloat("refractiveIndex", refractiveIndexes[currentRefractiveMaterial]);
#endif
	modelShaderProgram.SetVec3("cameraPos", camera.Position);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	modelShaderProgram.SetInt("skybox", 3);
	this->model.Draw(modelShaderProgram);

	// cube
	glm::mat4 cubeModel = glm::mat4(1.0f);
	cubeModel = glm::translate(cubeModel, glm::vec3(-2.0f, -1.75f, 0.0f));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMapTexture);	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularMapTexture);
	modelShaderProgram.SetInt("material.texture_diffuse1", 0);
	modelShaderProgram.SetInt("material.texture_specular1", 1);
	modelShaderProgram.SetMat4("model", cubeModel);
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthFunc(GL_LEQUAL);
	const glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix()));
	cubemapShader.Bind();
	cubemapShader.SetMat4("projection", projection);
	cubemapShader.SetMat4("view", skyboxView);
	glBindVertexArray(cubeMapVAO);

	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}

void HelloCubemapsDrawingProgram::Destroy()
{
}

void HelloCubemapsDrawingProgram::UpdateUi()
{
	ImGui::Separator();
	ImGui::SliderFloat("Reflection Value", &reflectionValue, 0.0f, 1.0f);
#ifdef REFRACTION
	ImGui::Combo("combo", &currentRefractiveMaterial, refractriveMaterialName, IM_ARRAYSIZE(refractriveMaterialName));
#endif
}

int main(int argc, char** argv)
{
	Engine engine;
	auto& config = engine.GetConfiguration();
	config.screenWidth = 1024;
	config.screenHeight = 1024;
	config.windowName = "Hello Cubemaps";
	engine.AddDrawingProgram(new HelloCubemapsDrawingProgram());

	engine.Init();
	engine.GameLoop();

	return EXIT_SUCCESS;
}