
#include <iostream>

#include <GL/glew.h>
#include <engine.h>
#include <graphics.h>
#ifdef USE_EMSCRIPTEN
#include <emscripten.h> 
#endif
#include <chrono>
#include "imgui.h"
#ifdef USE_SDL2
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#endif
#include <Remotery.h>

Engine* Engine::enginePtr = nullptr;

#ifdef WIN32
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

extern "C"
{
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
	fflush(stderr);
}

Engine::Engine()
{
	enginePtr = this;
}

Engine::~Engine()
{
	for(auto* drawingProgram : drawingPrograms)
	{
		drawingProgram->Destroy();
		delete drawingProgram;
	}
	drawingPrograms.clear();
}

void Engine::Init()
{

	rmt_CreateGlobalInstance(&rmt);
#ifdef USE_SDL2
	SDL_Init(SDL_INIT_VIDEO);
	// Set our OpenGL version.
// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, configuration.glMajorVersion);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, configuration.glMinorVersion);


	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	window = SDL_CreateWindow(
		configuration.windowName.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		configuration.screenWidth,
		configuration.screenHeight,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	// Check that everything worked out okay
	if (window == nullptr)
	{
		std::cerr << "Unable to create window\n";
		return;
	}


	glContext = SDL_GL_CreateContext(window);

	const GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cerr << "Error loading GLEW: " << glewGetErrorString(err) << "\n";
	}
#if _DEBUG == 1
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
#endif
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(window, glContext);
	ImGui_ImplOpenGL3_Init("#version 450");

	engineStartTime = timer.now();
	previousFrameTime = engineStartTime;

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), window);
#endif
	
	for (auto drawingProgram : drawingPrograms)
	{
		drawingProgram->Init();
	}
	glClearColor(configuration.bgColor.r, configuration.bgColor.g, configuration.bgColor.b, configuration.bgColor.a);

	SDL_GL_SetSwapInterval(configuration.vsync);
}


#ifdef USE_SDL2

void Engine::Loop()
{
	rmt_ScopedOpenGLSample(EngineLoopGPU);
	rmt_ScopedCPUSample(EngineLoopCPU, 0);
	std::chrono::high_resolution_clock::time_point currentFrame = timer.now();

	dt = std::chrono::duration_cast<ms>(currentFrame - previousFrameTime).count() / 1000.0f;
	previousFrameTime = currentFrame;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (enableImGui)
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
		}
		if (event.type == SDL_QUIT)
		{
			running = false;
		}
		if (event.type == SDL_WINDOWEVENT) 
		{
			if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				std::cout << "Window Size: " << event.window.data1 << ", " << event.window.data2 << "\n";
				Vec2f newWindowSize = Vec2f(event.window.data1, event.window.data2);
				std::cout << "New Window Size: " << newWindowSize << "\n";
				glViewport(0, 0, newWindowSize.x, newWindowSize.y);
				configuration.screenWidth = event.window.data1;
				configuration.screenHeight = event.window.data2;
			}
		}
		if(event.type == SDL_KEYDOWN)
		{
			switch(event.key.keysym.scancode)
			{
			case SDL_SCANCODE_1:
				SwitchWireframeMode();
				break;
			case SDL_SCANCODE_LALT:
				camera.SwitchWrapMode();
				break;
			default:
				break;
			}
		}
	}
	if (enableImGui)
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		UpdateUi();
	}
	ImGui::Render();
	SDL_GL_MakeCurrent(window, glContext);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(wireframeMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK,  GL_LINE);
	}
	for (auto drawingProgram : drawingPrograms)
	{
		drawingProgram->Draw();
	}
	if (enableImGui)
	{
		rmt_ScopedOpenGLSample(RenderImGuiGPU);
		rmt_ScopedCPUSample(RenderImGuiCPU, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	SDL_GL_SwapWindow(window);
}


void Engine::GameLoop()
{
	rmt_BindOpenGL();
	running = true;
	engineStartTime = timer.now();
	previousFrameTime = engineStartTime;

#ifdef __EMSCRIPTEN__
	// void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);
	emscripten_set_main_loop(Loop, 60,1);
#else
	while (running)
	{
		Loop();
	}
#endif
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	// Delete our OpengL context
	SDL_GL_DeleteContext(glContext);

	// Destroy our window
	SDL_DestroyWindow(window);

	// Shutdown SDL 2
	SDL_Quit();


}
#endif
void Engine::UpdateUi()
{
	rmt_ScopedOpenGLSample(DrawImGuiGPU);
	rmt_ScopedCPUSample(DrawImGuiCPU, 0);
	auto& config = GetConfiguration();
	const auto windowSize = Vec2f(config.screenWidth, config.screenHeight);
	if (debugInfo)
	{
#ifdef USE_SDL2

		int majorVersion = 0, minorVersion = 0;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &majorVersion);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minorVersion);
		ImGui::SetNextWindowPos(ImVec2(150, 0), ImGuiCond_Always);
		ImGui::Begin("Debug Info");
		ImGui::Text("OpenGL version: %d.%d", majorVersion, minorVersion);
		ImGui::Text("FPS: %4.0f", 1.0f / GetDeltaTime());
		ImGui::End();
#endif
	}

	if(drawingProgramsHierarchy)
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(150.0f, config.screenHeight), ImGuiCond_Always);
		ImGui::Begin("Hierarchy");
		for (int i = 0; i < drawingPrograms.size();i++)
		{
			auto* drawingProgram = drawingPrograms[i];
			if (ImGui::Selectable(drawingProgram->GetProgramName().c_str(), selectedDrawingProgram == i))
			{
				selectedDrawingProgram = i;
			}
		}
		ImGui::End();
	}

	if(inspector)
	{
		ImGui::SetNextWindowPos(ImVec2(windowSize.x - 250.0f, 0), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(250.0f, config.screenHeight), ImGuiCond_Always);
		ImGui::Begin("Inspector");
		if(selectedDrawingProgram != -1)
		{
			auto* drawingProgram = drawingPrograms[selectedDrawingProgram];
			auto& shaders = drawingProgram->GetShaders();
			for(int i = 0; i < shaders.size();i++)
			{
				GLint count;

				GLint size; // size of the variable
				GLenum type; // type of the variable (float, vec3 or mat4, etc)

				const GLsizei bufSize = 16; // maximum name length
				GLchar name[bufSize]; // variable name in GLSL
				GLsizei length; // name length

				auto* shader = shaders[i];
				ImGui::Text("Shader %d", i);
				glGetProgramiv(shader->GetProgram(), GL_ACTIVE_ATTRIBUTES, &count);

				for (int j = 0; j < count; j++)
				{
					glGetActiveAttrib(shader->GetProgram(), (GLuint)j, bufSize, &length, &size, &type, name);

					ImGui::Text("Attribute #%d Type: %u Name: %s", j, type, name);
				}

				glGetProgramiv(shader->GetProgram(), GL_ACTIVE_UNIFORMS, &count);

				for (int j = 0; j < count; j++)
				{
					glGetActiveUniform(shader->GetProgram(), (GLuint)j, bufSize, &length, &size, &type, name);

					ImGui::Text("Uniform #%d Type: %u Name: %s", j, type, name);
				}
			}
			drawingProgram->UpdateUi();

		}
		ImGui::End();
	}
}

float Engine::GetDeltaTime()
{
#ifdef USE_SDL2
	return dt;
#endif
}

float Engine::GetTimeSinceInit()
{
#ifdef USE_SDL2
	return std::chrono::duration_cast<ms>(previousFrameTime - engineStartTime).count() / 1000.f;
#endif
}

void Engine::AddDrawingProgram(DrawingProgram* drawingProgram)
{
	drawingPrograms.push_back(drawingProgram);
}

Engine* Engine::GetPtr()
{
	return enginePtr;
}


void Engine::SwitchWireframeMode()
{
	glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_FILL : GL_LINE);
	wireframeMode = !wireframeMode;
	
}

Configuration &Engine::GetConfiguration()
{
	return configuration;
}

InputManager& Engine::GetInputManager()
{
	return inputManager;
}

Camera& Engine::GetCamera()
{
	return camera;
}