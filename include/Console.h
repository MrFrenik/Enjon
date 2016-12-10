#ifndef ENJON_CONSOLE_H
#define ENJON_CONSOLE_H
#pragma once

#include <vector>

#include "GUI/GUI.h"

namespace Enjon {

	// Forward declaration
	namespace Input {
		class InputManager;
	}

	namespace Graphics {
		class Camera;
	}

	class Console
	{
		public:
			static void Init(float ScreenWidth, float ScreenHeight);
			static bool ProcessInput(Input::InputManager* Input);
			static void Update(float DT);
			static void Draw();

		public:
			static GUI::GUITextBox InputTextBox;
	};
}

#endif