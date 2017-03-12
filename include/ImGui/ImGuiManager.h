#pragma once
#ifndef ENJON_IM_GUI_MANAGER_H
#define ENJON_IM_GUI_MANAGER_H


#include "ImGui/imgui_impl_sdl_gl3.h"
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
	namespace Math
	{
		class Vec4;
	}

	namespace Graphics
	{
		class Window;
	}

	/*
		Static class meant to be a central hub for registering 
		ImGui commands
	*/
	class ImGuiManager
	{
		public:
			static void Init(SDL_Window* window);
			static void LateInit(SDL_Window* window);
			static void Register(std::function<void()> func);
			static void RegisterMenuOption(std::string name, std::function<void()> func);
			static void RegisterWindow(std::function<void()> func);
			static void RegisterDockingLayout(ImGui::DockingLayout& layout);
			static void RenderGameUI(Enjon::Graphics::Window* window, f32* view, f32* projection);
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
			static std::vector<ImGui::DockingLayout> mDockingLayouts;
	};
}

#endif