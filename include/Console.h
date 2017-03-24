#ifndef ENJON_CONSOLE_H
#define ENJON_CONSOLE_H
#pragma once

#include <vector>

#include "GUI/GUI.h"

namespace Enjon {

	// Forward declaration
	class Input;

	class Camera;

	class Console
	{
		public:
			static void Init(float ScreenWidth, float ScreenHeight);
			static bool ProcessInput(Input* sInput);
			static void Update(float DT);
			static void Draw();
			static bool Visible();
			static void Visible(bool visible);

		public:
			static GUI::GUITextBox InputTextBox;
			static bool mIsVisible;
	};
}

#endif