#pragma once
#include <string>
#include <vector>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <functional>


struct BasicMaterial
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;
};

class Shader
{
public:
	void CompileSource(std::string vertexShaderPath, std::string fragmentShaderPath);
	void CompileSpirV(std::string vertexShaderPath, std::string fragmentShaderPath);
	void Bind();
	int GetProgram();
	void SetBool(const std::string& attributeName, bool value) const;
	void SetInt(const std::string& attributeName, int value) const;
	void SetFloat(const std::string& attributeName, float value) const;
	void SetVec2(const std::string& name, float x, float y) const;
	void SetVec2(const std::string &name, const glm::vec2 &value) const;
	void SetVec3(const std::string& name, float x, float y, float z) const;
	void SetVec3(const std::string &name, const glm::vec3 &value) const;
	void SetVec3(const std::string &name, const float value[3]) const;
	void SetVec4(const std::string& name, float x, float y, float z, float w);
	void SetVec4(const std::string &name, const glm::vec4 &value) const;
	void SetMat2(const std::string &name, const glm::mat2 &mat) const;
	void SetMat3(const std::string &name, const glm::mat3 &mat) const;
	void SetMat4(const std::string &name, const glm::mat4 &mat) const;
	void SetBasicMaterial(const BasicMaterial& basicMaterial);
	void SetBindingFunction(std::function<void(void)> bindingFunction);
private:
	int shaderProgram = 0;
	std::function<void(void)> bindingFunction = nullptr;
};

class DrawingProgram
{
public:
	virtual ~DrawingProgram() = default;
	virtual void Init() = 0;
	virtual void Draw() = 0;
	virtual void Destroy() = 0;
	virtual void UpdateUi() {};
	const std::string& GetProgramName();
	const std::vector<Shader*>& GetShaders();

protected:
	virtual void ProcessInput();
	std::vector<Shader*> shaders;
	std::string programName;
};

class Skybox
{
public:
	void Init(std::vector<std::string>& faces);
	void Draw();
	void SetViewMatrix(const glm::mat4& view);
	void SetProjectionMatrix(glm::mat4& projection) { this->projection = projection; }
private:
	Shader skyboxShader;
	unsigned int cubemapTexture;
	Shader cubemapShader;
	unsigned cubeMapVAO;
	unsigned cubeMapVBO;
	glm::mat4 skyboxView;
	glm::mat4 projection;

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
};

unsigned int gliCreateTexture(char const* filename);
unsigned int stbCreateTexture(const char* filename, bool smooth = true, bool mipMaps = true, bool clampWrap=false);

unsigned int LoadCubemap(std::vector<std::string>& faces);