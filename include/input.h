#pragma once

#include <vector>
#include "vector.h"

#include <SDL.h>

class InputManager
{
public:
	explicit InputManager();
	
	bool GetButtonDown(SDL_Keycode key);
	bool GetButtonUp(SDL_Keycode key);
	bool GetButton(SDL_Keycode key);

	Vec2f GetMousePosition();
	float GetMouseWheelDelta();
private:
	friend class Engine;
	void Update();
	std::vector<char> previousButtonStatus;
	float wheelDelta;
};
