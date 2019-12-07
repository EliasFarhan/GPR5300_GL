#include <engine.h>
#include <graphics.h>



//#define OTHER_TEXTURE

class HelloTextureDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
private:

	float vertices[12] = {
		 0.5f,  0.5f, 0.0f,  // top right
		 0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
	};
	float texCoords[8] = {
		1.0f, 1.0f,	  // top right
		1.0f, 0.0f,   // bottom right
		0.0f, 0.0f,   // bottom left
		0.0f, 1.0f,   // bottom left
	};
	unsigned int indices[6] = {
		// note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	Shader shaderProgram;
	unsigned int VBO[2] = {}; //Vertex Buffer Object
	unsigned int VAO = 0; //Vertex Array Object
	unsigned int EBO = 0; // Element Buffer Object
	GLuint textureWall;
#ifdef OTHER_TEXTURE
	GLuint textureOtherPlay;
#endif
};

void HelloTextureDrawingProgram::Init()
{

	programName = "HelloTexture";

	shaders.push_back(&shaderProgram);

	glGenBuffers(2, &VBO[0]);
	glGenBuffers(1, &EBO);

    shaderProgram.CompileSource(
            "shaders/02_hello_texture/texture.vert",
#ifdef OTHER_TEXTURE
            "shaders/02_hello_texture/texture_other.frag"
#else
            "shaders/02_hello_texture/texture.frag"
#endif
    );
	textureWall = stbCreateTexture("data/sprites/wall.jpg");
#ifdef OTHER_TEXTURE
	textureOtherPlay = stbCreateTexture("data/sprites/other_play.png");
#endif
	glGenVertexArrays(1, &VAO);
	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);
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
}

void HelloTextureDrawingProgram::Draw()
{
	shaderProgram.Bind();
#ifdef OTHER_TEXTURE
	glUniform1i(glGetUniformLocation(shaderProgram.GetProgram(), "texture1"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram.GetProgram(), "texture2"), 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureOtherPlay);
#else
	glBindTexture(GL_TEXTURE_2D, textureWall);
#endif
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	
}

void HelloTextureDrawingProgram::Destroy()
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
	//config.bgColor = sf::Color::Black;
	config.windowName = "Hello Texture";

	engine.AddDrawingProgram(new HelloTextureDrawingProgram());
	engine.Init();
	engine.GameLoop();
	return EXIT_SUCCESS;
}
