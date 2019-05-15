#pragma once
#include <vector>

class Plane
{
public:
	void Init();
	void Draw() const;
private:
	std::vector<float> vertices;
	unsigned quadVAO;
	unsigned quadVBO;
};

class Cube
{
public:
	void Init();
	void Draw();
private:
	std::vector<float> vertices;
	unsigned cubeVAO;
	unsigned cubeVBO;
};

class Sphere
{
public:
    void Init();
    void Draw();
private:
    std::vector<float> vertices;
	std::vector<unsigned int> indices;
    unsigned sphereVAO = 0;
    unsigned sphereVBO = 0;
    unsigned sphereEBO = 0;

    unsigned int indexCount;

};