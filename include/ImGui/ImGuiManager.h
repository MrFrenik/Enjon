#pragma once
#ifndef ENJON_IM_GUI_MANAGER_H
#define ENJON_IM_GUI_MANAGER_H

#include "ImGui/imgui.h"

#include <functional>
#include <vector>

namespace Enjon
{
	/*
		Static class meant to be a central hub for registering 
		ImGui commands
	*/
	class ImGuiManager
	{
		public:
			static void Register(std::function<void()> func);
			static void Render();

		private:
			static std::vector<std::function<void()>> mGuiFuncs;
	};
}

#endif