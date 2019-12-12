
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <gl_wrappers.h>
#include <SDL.h>
#include <engine.h>
#include <graphics.h>


#include <chrono>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#ifndef __EMSCRIPTEN__
#include <Remotery.h>
#endif
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
#ifndef __EMSCRIPTEN__
	rmt_CreateGlobalInstance(&rmt);
#endif
	SDL_Init(SDL_INIT_VIDEO);
	// Set our OpenGL version.
// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

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


    if (!gladLoadGLES2Loader((GLADloadproc) SDL_GL_GetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        assert(false);
    }

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
	ImGui_ImplOpenGL3_Init("#version 300 es");

	engineStartTime = timer.now();
	previousFrameTime = engineStartTime;

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), window);
	
	for (auto drawingProgram : drawingPrograms)
	{
		drawingProgram->Init();
	}
	glClearColor(configuration.bgColor.r, configuration.bgColor.g, configuration.bgColor.b, configuration.bgColor.a);

	SDL_GL_SetSwapInterval(configuration.vsync);
}


void Engine::Loop()
{
#ifndef __EMSCRIPTEN__
	rmt_ScopedOpenGLSample(EngineLoopGPU);
	rmt_ScopedCPUSample(EngineLoopCPU, 0);
#endif
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

	for (auto drawingProgram : drawingPrograms)
	{
		drawingProgram->Draw();
	}
	if (enableImGui)
	{
#ifndef __EMSCRIPTEN__
		rmt_ScopedOpenGLSample(RenderImGuiGPU);
		rmt_ScopedCPUSample(RenderImGuiCPU, 0);
#endif
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	SDL_GL_SwapWindow(window);
}

#ifdef __EMSCRIPTEN__
void EmLoop(void* engine)
{
  static_cast<Engine*>(engine)->Loop();
}
#endif
void Engine::GameLoop()
{
#ifndef __EMSCRIPTEN__
	rmt_BindOpenGL();
#endif
	running = true;
	engineStartTime = timer.now();
	previousFrameTime = engineStartTime;

#ifdef __EMSCRIPTEN__
	// void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);
	emscripten_set_main_loop_arg(&EmLoop, this, 0,0);
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

void Engine::UpdateUi()
{
#ifndef __EMSCRIPTEN__
	rmt_ScopedOpenGLSample(DrawImGuiGPU);
	rmt_ScopedCPUSample(DrawImGuiCPU, 0);
#endif
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

			drawingProgram->UpdateUi();
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

		}
		ImGui::End();
	}
}

float Engine::GetDeltaTime()
{
	return dt;
}

float Engine::GetTimeSinceInit()
{
	return std::chrono::duration_cast<ms>(previousFrameTime - engineStartTime).count() / 1000.f;
}

void Engine::AddDrawingProgram(DrawingProgram* drawingProgram)
{
	drawingPrograms.push_back(drawingProgram);
}

Engine* Engine::GetPtr()
{
	return enginePtr;
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