#pragma once
#ifndef ENJON_IM_GUI_MANAGER_H
#define ENJON_IM_GUI_MANAGER_H

#include "ImGui/imgui.h"
#include "ImGui/imgui_dock.h"
#include "ImGui/ImGuizmo.h"
#include "System/Types.h"

#include <SDL2/sdl.h>

#include <functional>
#include <unordered_map>
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
			static void Init(SDL_Window* window);
			static void Register(std::function<void()> func);
			static void RegisterMenuOption(std::string name, std::function<void()> func);
			static void RegisterWindow(std::function<void()> func);
			static void RenderGameUI(SDL_Window* window, f32* view, f32* projection);
			static void Render(SDL_Window* window);
			static void ShutDown();

		private:
			static s32 MainMenu();
			static void ImGuiStyles();
			static void InitializeDefaults();

		private:
			static std::vector<std::function<void()>> mGuiFuncs;
			static std::vector<std::function<void()>> mWindows;
			static std::unordered_map<std::string, std::vector<std::function<void()>>> mMainMenuOptions;
	};
}

#endif