#include <input.h>
#include <engine.h>

InputManager::InputManager()
{
}
bool InputManager::GetButtonDown(SDL_Keycode key)
{
	return false;
}
bool InputManager::GetButtonUp(SDL_Keycode key)
{
	return false;
}
bool InputManager::GetButton(SDL_Keycode key)
{
	const auto scanCode = SDL_GetScancodeFromKey(key);
	const auto* keyState = SDL_GetKeyboardState(NULL);
	return keyState[scanCode];
}

Vec2f InputManager::GetMousePosition()
{
	int x, y;
	SDL_GetMouseState(&x, &y);
	return Vec2f(x, y);
}
float InputManager::GetMouseWheelDelta()
{
	return wheelDelta;
}

