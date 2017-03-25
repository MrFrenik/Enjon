#include "ImGui/ImGuiManager.h"
#include "ImGui/imgui_impl_sdl_gl3.h"
#include "Graphics/Camera.h"
#include "Graphics/Window.h"
#include "System/Types.h"
#include "Defines.h"

#include <algorithm>
#include <assert.h>

namespace Enjon
{
	std::vector<std::function<void()>> ImGuiManager::mGuiFuncs;
	std::vector<std::function<void()>> ImGuiManager::mWindows;
	std::unordered_map<std::string, std::vector<std::function<void()>>> ImGuiManager::mMainMenuOptions;
	std::vector<ImGui::DockingLayout> ImGuiManager::mDockingLayouts;

	//---------------------------------------------------
	void ImGuiManager::Init(SDL_Window* window)
	{
		assert(window != nullptr);

		// Init window
		ImGui_ImplSdlGL3_Init(window); 

		// Init style
		ImGuiStyles();

		// Initialize default windows/menus
		InitializeDefaults();
	}

	void ImGuiManager::ShutDown()
	{
		// Save dock
		// ImGui::SaveDock();

		// Shut down 
		ImGui_ImplSdlGL3_Shutdown();
	}

	//---------------------------------------------------
	void ImGuiManager::RegisterMenuOption(std::string name, std::function<void()> func)
	{
		// Will create the vector if not there
		mMainMenuOptions[name].push_back(func);
	}

	//---------------------------------------------------
	void ImGuiManager::Register(std::function<void()> func)
	{
		// TODO(): Search for function first before adding
		mGuiFuncs.push_back(func);
	}

	//---------------------------------------------------
	void ImGuiManager::RegisterWindow(std::function<void()> func)
	{
		mWindows.push_back(func);
	}

	void ImGuiManager::RenderGameUI(Window* window, f32* view, f32* projection)
	{
	    // Make a new window
		// ImGui_ImplSdlGL3_NewFrame(window);

		// Original screen coords
		auto dimensions = window->GetViewport();
		Enjon::Vec2 center = Enjon::Vec2((f32)dimensions.x / 2.0f, (f32)dimensions.y / 2.0f);

	    ImGuizmo::BeginFrame();
		static Mat4 model = Mat4::Identity();
		Enjon::Vec2 translate(0, 0);
    	ImGui::SliderFloat("Translate X", &translate.x, 0.0f, 1.0f);     // adjust display_format to decorate the value with a prefix or a suffix. Use power!=1.0 for logarithmic sliders
    	ImGui::SliderFloat("Translate Y", &translate.y, 0.0f, 1.0f);     // adjust display_format to decorate the value with a prefix or a suffix. Use power!=1.0 for logarithmic sliders

    	Vec3 scale(1, 1, 1);
    	ImGui::SliderFloat("Scale X", &scale.x, 0.01f, 1.0);     // adjust display_format to decorate the value with a prefix or a suffix. Use power!=1.0 for logarithmic sliders
    	ImGui::SliderFloat("Scale Y", &scale.y, 0.01f, 1.0);     // adjust display_format to decorate the value with a prefix or a suffix. Use power!=1.0 for logarithmic sliders
    	ImGui::SliderFloat("Scale Z", &scale.z, 0.01f, 1.0);     // adjust display_format to decorate the value with a prefix or a suffix. Use power!=1.0 for logarithmic sliders

    	model *= Mat4::Translate(Vec3(translate, 0.0f));
    	model *= Mat4::Scale(scale);

	    // Imguizmo 
	    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
	    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
	    if (ImGui::IsKeyPressed(90))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		if (ImGui::IsKeyPressed(69))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		if (ImGui::IsKeyPressed(82)) // r Key
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.elements, matrixTranslation, matrixRotation, matrixScale);
		ImGui::InputFloat3("Tr", matrixTranslation, 3);
		ImGui::InputFloat3("Rt", matrixRotation, 3);
		ImGui::InputFloat3("Sc", matrixScale, 3);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.elements);

		if (mCurrentGizmoOperation != ImGuizmo::SCALE)
		{
			if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
				mCurrentGizmoMode = ImGuizmo::LOCAL;
			ImGui::SameLine();
			if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
				mCurrentGizmoMode = ImGuizmo::WORLD;
		}
		static bool useSnap(false);
		if (ImGui::IsKeyPressed(83))
			useSnap = !useSnap;
		ImGui::Checkbox("", &useSnap);
		ImGui::SameLine();
		Enjon::Vec2 snap;

		/*
		switch (mCurrentGizmoOperation)
		{
		case ImGuizmo::TRANSLATE:
			snap = config.mSnapTranslation;
			ImGui::InputFloat3("Snap", &snap.x);
			break;
		case ImGuizmo::ROTATE:
			snap = config.mSnapRotation;
			ImGui::InputFloat("Angle Snap", &snap.x);
			break;
		case ImGuizmo::SCALE:
			snap = config.mSnapScale;
			ImGui::InputFloat("Scale Snap", &snap.x);
			break;
		}
		*/

		ImGuizmo::Manipulate(view, projection, mCurrentGizmoOperation, mCurrentGizmoMode, model.elements, NULL, useSnap ? &snap.x : NULL);
	}

	//---------------------------------------------------
	void ImGuiManager::Render(SDL_Window* window)
	{
	    // Make a new window
		ImGui_ImplSdlGL3_NewFrame(window);

		static bool show_scene1 = true;

		s32 menu_height = MainMenu();

	    if (ImGui::GetIO().DisplaySize.y > 0) {
	        ////////////////////////////////////////////////////
	        // Setup root docking window                      //
	        // taking into account menu height and status bar //
	        ////////////////////////////////////////////////////
	        auto pos = ImVec2(0, menu_height);
	        auto size = ImGui::GetIO().DisplaySize;
	        size.y -= pos.y;
	        ImGui::RootDock(pos, ImVec2(size.x, size.y - 25.0f));

	        // Draw status bar (no docking)
	        ImGui::SetNextWindowSize(ImVec2(size.x, 25.0f), ImGuiSetCond_Always);
	        ImGui::SetNextWindowPos(ImVec2(0, size.y - 6.0f), ImGuiSetCond_Always);
	        ImGui::Begin("statusbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize);
	        ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
	        ImGui::End();
	    }

	    // Display all registered windows
	    for (auto& wind : mWindows)
	    {
	    	wind();
	    }
	}

	//---------------------------------------------------
	void ImGuiManager::RegisterDockingLayout(ImGui::DockingLayout& layout)
	{
		mDockingLayouts.push_back(layout);
	}

	//---------------------------------------------------
	void ImGuiManager::LateInit(SDL_Window* window)
	{
		Render(window);

		// Run through docking layouts here
    	for (auto& dl : mDockingLayouts)
    	{
    		ImGui::DockWith(dl.mChild, dl.mParent, dl.mSlotType, dl.mWeight);
    	}

    	// Clear docking layouts after to prevent from running through them again
    	mDockingLayouts.clear();
	}

	//---------------------------------------------------
	s32 ImGuiManager::MainMenu()
	{
		s32 menuHeight = 0;
		if (ImGui::BeginMainMenuBar())
		{
			// Display all menu options
			if (ImGui::BeginMenu("File"))
			{
				for (auto& sub : mMainMenuOptions["File"])
				{
					sub();		
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				for (auto& sub : mMainMenuOptions["View"])
				{
					sub();		
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				for (auto& sub : mMainMenuOptions["Help"])
				{
					sub();		
				}
				ImGui::EndMenu();
			}

			menuHeight = ImGui::GetWindowSize().y;

			ImGui::EndMainMenuBar();
		}

		return menuHeight;
	} 

	//------------------------------------------------------------------------------
	void ImGuiManager::ImGuiStyles()
	{
		ImGuiIO& io = ImGui::GetIO();

		io.Fonts->Clear();
		io.Fonts->AddFontFromFileTTF("../Assets/Fonts/WeblySleek/weblysleekuisb.ttf", 16);
		io.Fonts->AddFontFromFileTTF("../Assets/Fonts/WeblySleek/weblysleekuisb.ttf", 14);
		io.Fonts->Build();

		// Grab reference to style
		ImGuiStyle& style = ImGui::GetStyle();

		// style.WindowPadding            = ImVec2(3, 7);
		// style.WindowRounding           = 2.0f;
		// style.FramePadding             = ImVec2(2, 0);
		// style.FrameRounding            = 2.0f;
		// style.ItemSpacing              = ImVec2(8, 4);
		// style.ItemInnerSpacing         = ImVec2(2, 2);
		// style.IndentSpacing            = 21.0f;
		// style.ScrollbarSize            = 11.0f;
		// style.ScrollbarRounding        = 9.0f;
		// style.GrabMinSize              = 4.0f;
		// style.GrabRounding             = 3.0f;
		style.WindowTitleAlign 		   = ImVec2(0.5f, 0.41f);
		style.ButtonTextAlign 		   = ImVec2(0.5f, 0.5f);
		// style.Alpha = 1.0f;
  //       style.FrameRounding = 3.0f;

  //      	style.Colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 0.71f);
		// style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		// style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		// style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		// style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.04f, 0.04f, 0.04f, 0.94f);
		// style.Colors[ImGuiCol_Border]                = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
		// style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		// style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		// style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.43f, 0.43f, 0.43f, 0.17f);
		// style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		// style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		// style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		// style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		// style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		// style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		// style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		// style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		// style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		// style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		// style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		// style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		// style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		// style.Colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.20f, 0.20f, 0.40f);
		// style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.22f, 0.22f, 0.22f, 0.85f);
		// style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.24f, 0.24f, 0.24f, 0.63f);
		// style.Colors[ImGuiCol_Header]                = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		// style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		// style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		// style.Colors[ImGuiCol_Column]                = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		// style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
		// style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		// style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		// style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		// style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.93f);
		// style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
		// style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		// style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		// style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		// style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		// style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		// style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		// style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		// style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		style.WindowPadding = ImVec2(6, 4);
		style.WindowRounding = 2.0f;
		style.FramePadding = ImVec2(7, 5);
		style.FrameRounding = 2.0f;
		style.ItemSpacing = ImVec2(8, 4);
		style.ItemInnerSpacing = ImVec2(2, 2);
		style.IndentSpacing = 21.0f;
		style.ScrollbarSize = 15.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabMinSize = 20.0f;
		style.GrabRounding = 3.0f;
		style.Alpha = 1.0f;
		style.FrameRounding = 3.0f;

		// style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 0.61f);
		// style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 0.81f);
		// style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.11f, 0.15f, 1.00f);
		// style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.09f, 0.11f, 0.17f, 1.00f);
		// style.Colors[ImGuiCol_PopupBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		// style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		// style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.39f, 0.61f, 1.00f, 0.26f);
		// style.Colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.08f, 1.00f);
		// style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		// style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		// style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 0.00f);
		// style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		// style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.00f);
		// style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		// style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		// style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		// style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		// style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		// style.Colors[ImGuiCol_ComboBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		// style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		// style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		// style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		// style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 0.40f);
		// style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.22f, 0.22f, 0.85f);
		// style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.24f, 0.24f, 0.51f);
		// style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.68f, 0.26f, 0.08f, 1.00f);
		// style.Colors[ImGuiCol_Header] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		// style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		// style.Colors[ImGuiCol_Column] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		// style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
		// style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		// style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		// style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		// style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.93f);
		// style.Colors[ImGuiCol_CloseButton] = ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
		// style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		// style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		// style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		// style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		// style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		// style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		// style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		// style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		style.Colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 0.61f);
		style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.40f, 0.40f, 0.40f, 0.81f);
		style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.09f, 0.11f, 0.13f, 1.00f);
		style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.09f, 0.11f, 0.13f, 0.31f);
		style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		style.Colors[ImGuiCol_Border]                = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.39f, 0.61f, 1.00f, 0.18f);
		style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.06f, 0.06f, 0.08f, 0.67f);
		style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.09f, 0.11f, 0.13f, 1.00f);
		style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.04f, 0.04f, 0.04f, 0.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.18f, 0.18f, 0.18f, 0.00f);
		style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.04f, 0.05f, 0.06f, 0.99f);
		style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.06f, 0.06f, 0.06f, 0.87f);
		style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.14f, 0.14f, 0.14f, 0.85f);
		style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.20f, 0.20f, 0.52f);
		style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.24f, 0.24f, 0.24f, 0.51f);
		style.Colors[ImGuiCol_Header]                = ImVec4(0.06f, 0.06f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.26f, 0.59f, 1.00f, 0.52f);
		style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Column]                = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.93f);
		style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
		style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		// Load dock
		// ImGui::LoadDock();
	}

	//--------------------------------------------------
	void ImGuiManager::InitializeDefaults()
	{
		mMainMenuOptions["File"].push_back([&](){
			static bool on = false;
	    	ImGui::MenuItem("Save##file", NULL, &on);
		});
	}
}