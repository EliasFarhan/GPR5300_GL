#pragma once

#include <engine.h>
#include <graphics.h>
#include <mesh.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
	/*  Model Data */
	std::vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	std::vector<Mesh> meshes;
	std::string directory;
	bool gammaCorrection;
	glm::vec3 modelCenter;
	float modelRadius;
	/*  Functions   */
	void Init(const char *path, bool generateSphere=false)
	{
		loadModel(path, generateSphere);
	}
	void Draw(Shader& shader);
private:

	/*  Functions   */
	void loadModel(std::string path, bool generateSphere);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
		std::string typeName);
};