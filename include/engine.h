#pragma once

#include <functional>
#include <vector>
#include <string>

#ifndef __EMSCRIPTEN__
#include <glad/glad.h>
#endif
#include <SDL.h>
#ifdef WIN32
#include <SDL_main.h>
#endif

#include <chrono>
#include <input.h>
#include <camera.h>

class DrawingProgram;
struct Remotery;

typedef SDL_Color Color;
using ms = std::chrono::duration<float, std::milli>;

struct Configuration
{
	unsigned int screenWidth = 800;
	unsigned int screenHeight = 600;
	int vsync = 1;
	Color bgColor = {0,0,0,0};

	std::string windowName = "OpenGL";
	unsigned int glMajorVersion = 3;
	unsigned int glMinorVersion = 0;
};

class Engine
{
public:
	Engine();
	~Engine();
	void Init();
	void GameLoop();

	void UpdateUi();

	float GetDeltaTime();
	float GetTimeSinceInit();

	Configuration& GetConfiguration();
	InputManager& GetInputManager();
	Camera& GetCamera();
	void AddDrawingProgram(DrawingProgram* drawingProgram);
	std::vector<DrawingProgram*>& GetDrawingPrograms() { return drawingPrograms; };


	static Engine* GetPtr();
private:
	static Engine* enginePtr;


	void Loop();
	SDL_Window* window = nullptr;
	SDL_GLContext glContext;
	std::chrono::high_resolution_clock timer;

	std::chrono::high_resolution_clock::time_point engineStartTime;
	std::chrono::high_resolution_clock::time_point previousFrameTime;
	float dt;

	std::vector<DrawingProgram*> drawingPrograms;
	InputManager inputManager;
	Camera camera;
	Configuration configuration;
	Remotery* rmt;
	int selectedDrawingProgram = -1;
	bool debugInfo = true;
	bool drawingProgramsHierarchy = true;
	bool inspector = true;
	bool enableImGui = true;
	bool running = false;
};

