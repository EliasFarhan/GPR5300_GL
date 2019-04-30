#pragma once
#include <vector>

class Plane
{
public:
	void Init();
	void Draw();
private:
	std::vector<float> vertices;
	unsigned quadVAO;
	unsigned quadVBO;
};
