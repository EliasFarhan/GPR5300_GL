//
// Created by efarhan on 5/3/19.
//

#include <engine.h>
#include <graphics.h>

class HelloPBRDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;

};


int main(int argc, char** argv)
{
	Engine engine;
	srand(0);
	auto& config = engine.GetConfiguration();
	config.screenWidth = 1280;
	config.screenHeight = 720;
	config.windowName = "Hello PBR";
	config.bgColor = { 1,1,1,1 };
	engine.AddDrawingProgram(new HelloPBRDrawingProgram());
	engine.Init();
	engine.GameLoop();
	return EXIT_SUCCESS;
}
