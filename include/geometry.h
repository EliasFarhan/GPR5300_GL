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