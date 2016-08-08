/*-- Enjon includes --*/
#include "Editor/AnimationEditor.h"
#include "IO/InputManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/FontManager.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/CursorManager.h"
#include "GUI/GUIAnimationElement.h"
#include "Physics/AABB.h"
#include "Utils/Errors.h"
#include "Defines.h"

/*-- IsoARPG includes --*/
#include "EnjonAnimation.h"
#include "AnimationManager.h"
#include "AnimManager.h"
#include "SpatialHash.h"
#include "Level.h"

/*-- Standard Library includes --*/
#include <unordered_map>
#include <string>

/*-- 3rd Party Includes --*/
#include <SDL2/SDL.h>

using namespace EA;
using json = nlohmann::json;

namespace Enjon { namespace GUI {

	namespace GUIManager
	{
		std::unordered_map<std::string, GUIElementBase*> Elements;

		void Add(std::string S, GUIElementBase* E)
		{
			Elements[S] = E;
		}

		template <typename T>
		T* Get(const std::string S)
		{
			auto search = Elements.find(S);
			if (search != Elements.end())
			{
				return static_cast<T*>(search->second);
			}

			EU::FatalError(std::string("GUIElement does not exist: ") + S);
			return nullptr;
		}
	}
}}

namespace CameraManager
{
	std::unordered_map<std::string, EG::Camera2D*> Cameras;

	void AddCamera(std::string S, EG::Camera2D* C)
	{
		Cameras[S] = C;
	}

	EG::Camera2D* GetCamera(const std::string S)
	{
		auto search = Cameras.find(S);
		if (search != Cameras.end())
		{
			return search->second;
		}
		return nullptr;
	}
};

namespace Enjon { namespace AnimationEditor {

	using namespace GUI;
	using namespace Graphics;

	/*-- Function Delcarations --*/
	bool ProcessInput(EI::InputManager* Input);
	void CalculateAABBWithParent(EP::AABB* A, GUIButton* Button);
	bool IsModifier(unsigned int Key);
	bool HasChildren(GUIElementBase* E);

	/*-- Constants --*/
	const std::string AnimTextureDir("../IsoARPG/Assets/Textures/Animations/Player/Attack/OH_L/SE/Player_Attack_OH_L_SE.png");
	const std::string AnimTextureJSONDir("../IsoARPG/Assets/Textures/Animations/Player/Attack/OH_L/SE/Player_Attack_OH_L_SE.json");
	const std::string AnimationDir("../IsoARPG/Profiles/Animations/Player/PlayerAttackOHLSEAnimation.json");
	
	GUIElementBase* SelectedGUIElement = nullptr;
	GUIElementBase* KeyboardFocus = nullptr;
	GUIElementBase* MouseFocus = nullptr;

	EI::InputManager* Input = nullptr;

	////////////////////////////////
	// ANIMATION EDITOR ////////////

	GUIGroup					SceneGroup;
	GUIGroup					AnimationPanel;
	GUIButton 					PlayButton;
	GUIRadialButton				AnimationOnionSkin;
	GUIDropDownButton 			AnimationSelection;	
	GUIAnimationElement 		SceneAnimation;
	GUIValueButton<float>		AnimationDelay;
	GUIValueButton<float>		AnimationTimeScale;
	GUIValueButton<uint32_t>	AnimationFrame;
	GUIValueButton<int32_t>		AnimationXOffset;
	GUIValueButton<int32_t>		AnimationYOffset;
	
	
	Atlas atlas;
	float AWidth;
	float AHeight;
	
	float SCREENWIDTH;
	float SCREENHEIGHT;

	float caret_count = 0.0f;
	bool caret_on = true;
	float TimeScale = 1.0f;
	float TimeIncrement = 0.15f;
	float t = 0.0f;

	bool IsRunning = true;
	
	EG::GLSLProgram* BasicShader 	= nullptr;
	EG::GLSLProgram* TextShader 	= nullptr;
	EG::SpriteBatch* UIBatch 		= nullptr;
	EG::SpriteBatch* SceneBatch 	= nullptr;
	EG::SpriteBatch* BGBatch 		= nullptr;
	EG::SpriteBatch* UIObjectBatch 	= nullptr;

	EG::Camera2D Camera;
	EG::Camera2D HUDCamera;

	/*-- Function Definitions --*/
	bool Init(EI::InputManager* Input_Mgr, float SW, float SH)
	{
		// Shader for frame buffer
		BasicShader		= EG::ShaderManager::GetShader("Basic");
		TextShader		= EG::ShaderManager::GetShader("Text");  

		// UI Batch
		UIBatch = new EG::SpriteBatch();
		UIBatch->Init();

		SceneBatch = new EG::SpriteBatch();
		SceneBatch->Init();

		BGBatch = new EG::SpriteBatch();
		BGBatch->Init();

		UIObjectBatch = new EG::SpriteBatch();
		UIObjectBatch->Init();

		Input = Input_Mgr;

		const float W = SW;
		const float H = SH;

		SCREENWIDTH = SW;
		SCREENHEIGHT = SH;

		// Create Camera
		Camera.Init(W, H);

		// Create HUDCamera
		HUDCamera.Init(W, H);

		// Register cameras with manager
		CameraManager::AddCamera("HUDCamera", &HUDCamera);
		CameraManager::AddCamera("SceneCamera", &Camera);
	
		auto Json = EU::read_file_sstream(AnimTextureJSONDir.c_str());
	    
	   	// parse and serialize JSON
	   	json j_complete = json::parse(Json);

	   	// Get handle to frames data
	   	auto Frames = j_complete.at("frames");

	    // Get handle to meta deta
	    const auto Meta = j_complete.at("meta");

	    // Get image size
	    auto ISize = Meta.at("size");
	    AWidth = ISize.at("w");
	    AHeight = ISize.at("h");

	    atlas = {	
	    			EM::Vec2(AWidth, AHeight), 
					EI::ResourceManager::GetTexture(AnimTextureDir.c_str(), GL_LINEAR)
			  	};

		// Set up Scene Animtation
		SceneAnimation.Position = EM::Vec2(0.0f);
		SceneAnimation.CurrentAnimation = AnimManager::GetAnimation("Player_Attack_OH_L_SE");

		// Set up AnimationSelection's text with name of current animation
		AnimationSelection.Text = SceneAnimation.CurrentAnimation->Name;  // Can set up signal for when this changes to emit
		AnimationSelection.CalculateDimensions();

		// Add animations to AnimationSelection drop down button
		for (auto& a : *AnimManager::GetAnimationMap())
		{
			// Make button to be pushed back
			auto b = new GUITextButton();

			// Give name of button name of current animation
			b->Text = a.first;

			// Set up on_hover signal
			b->on_click.connect([&](GUIElementBase* b)
			{
				// Set Current animation to this
				SceneAnimation.CurrentAnimation = AnimManager::GetAnimation(b->Text);

				// Set text of animation selection
				AnimationSelection.Text = b->Text;

				// Deactive animation selection
				AnimationSelection.on_click.emit();

				AnimationSelection.CalculateDimensions();

				// Reset animation frame
				AnimationFrame.MaxValue = SceneAnimation.CurrentAnimation->Frames.size() - 1;
				AnimationFrame.Set(0);

				// Reset animation delay
				AnimationDelay.Set(SceneAnimation.CurrentAnimation->Frames.at(0).Delay);

				// Reset animation x offset
				AnimationXOffset.Set(SceneAnimation.CurrentAnimation->Frames.at(0).Offsets.x);

				// Reset animation y offset
				AnimationYOffset.Set(SceneAnimation.CurrentAnimation->Frames.at(0).Offsets.y);

				SceneAnimation.CurrentIndex = 0;
			});

			// Push back into drop down list
			AnimationSelection.List.push_back(b);
		}

		AnimationFrame.ValueText.TextFont = EG::FontManager::GetFont("WeblySleek_12");

		// Set up AnimationSelection AABB
		AnimationSelection.AABB.Min = AnimationSelection.Position;
		AnimationSelection.AABB.Max = AnimationSelection.AABB.Min + AnimationSelection.Dimensions;

		// Set up PlayButton's signal
		PlayButton.on_click.connect([&]()
		{
			if (PlayButton.State)
			{
				PlayButton.State = ButtonState::INACTIVE;
				auto SA = SceneAnimation.CurrentAnimation;
				AnimationFrame.Set(SceneAnimation.CurrentIndex);
				AnimationDelay.Set(SA->Frames.at(SceneAnimation.CurrentIndex).Delay);
				AnimationXOffset.Set(SA->Frames.at(SceneAnimation.CurrentIndex).Offsets.x);
				AnimationYOffset.Set(SA->Frames.at(SceneAnimation.CurrentIndex).Offsets.y);
			}
			else
			{
				PlayButton.State = ButtonState::ACTIVE;
			}
		});

		// Additional on click properties for buttons
		AnimationFrame.on_click.connect([&]()
		{
			if (PlayButton.State) PlayButton.on_click.emit();
		});

		AnimationDelay.on_click.connect([&]()
		{
			if (PlayButton.State) PlayButton.on_click.emit();
		});

		AnimationXOffset.on_click.connect([&]()
		{
			if (PlayButton.State) PlayButton.on_click.emit();
		});

		AnimationYOffset.on_click.connect([&]()
		{
			if (PlayButton.State) PlayButton.on_click.emit();
		});

		AnimationDelay.lose_focus.connect([&]()
		{
			auto V = AnimationFrame.Value.get();
			SceneAnimation.CurrentAnimation->Frames.at(V).Delay = AnimationDelay.Value.get();
		});

		AnimationFrame.lose_focus.connect([&]()
		{
			auto SA = SceneAnimation.CurrentAnimation;
			SceneAnimation.CurrentIndex = AnimationFrame.Value.get();
			AnimationDelay.Set(SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex).Delay);
			AnimationXOffset.Set(SA->Frames.at(SceneAnimation.CurrentIndex).Offsets.x);
			AnimationYOffset.Set(SA->Frames.at(SceneAnimation.CurrentIndex).Offsets.y);
		});

		AnimationXOffset.lose_focus.connect([&]()
		{
			SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex).Offsets.x = AnimationXOffset.Value.get();
		});

		AnimationYOffset.lose_focus.connect([&]()
		{
			SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex).Offsets.y = AnimationYOffset.Value.get();
		});

		AnimationTimeScale.lose_focus.connect([&]()
		{
			TimeScale = AnimationTimeScale.Value.get();
		});

		SceneAnimation.on_value_change.connect([&]()
		{
			// Reset offsets
			auto SA = SceneAnimation.CurrentAnimation;
			AnimationXOffset.Set(SA->Frames.at(SceneAnimation.CurrentIndex).Offsets.x);
			AnimationYOffset.Set(SA->Frames.at(SceneAnimation.CurrentIndex).Offsets.y);
		});

		// Add to GUIManager
		GUIManager::Add("PlayButton", &PlayButton);
		GUIManager::Add("AnimationPanel", &AnimationPanel);
		GUIManager::Add("SceneGroup", &SceneGroup);

		// Set up AnimationPanel Group
		AnimationPanel.Name = "Animation Editor";
		AnimationPanel.Position = EM::Vec2(150.0f, 0.0f);														// Set position of group

		AnimationDelay.Step = 0.1f;
		AnimationDelay.MinValue = 0.000001f;
		AnimationDelay.Set(SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex).Delay); 

		AnimationFrame.MaxValue = SceneAnimation.CurrentAnimation->Frames.size() - 1;
		AnimationFrame.MinValue = 0;
		AnimationFrame.Set(SceneAnimation.CurrentIndex);
		AnimationFrame.LoopValues = true;

		AnimationXOffset.MaxValue = SCREENWIDTH / 2.0f;
		AnimationXOffset.MinValue = -SCREENWIDTH / 2.0f;
		AnimationXOffset.Step = 1;
		AnimationXOffset.Set(SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex).Offsets.x);

		AnimationYOffset.MaxValue = SCREENHEIGHT / 2.0f;
		AnimationYOffset.MinValue = -SCREENHEIGHT / 2.0f;
		AnimationYOffset.Step = 1;
		AnimationYOffset.Set(SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex).Offsets.y);

		AnimationTimeScale.MaxValue = 1.0f;
		AnimationTimeScale.MinValue = 0.000001f;
		AnimationTimeScale.Step = 0.1f;
		AnimationTimeScale.Set(1.0f);

		AnimationPanel.AddToGroup(&AnimationSelection, "Animation");
		AnimationPanel.AddToGroup(&AnimationFrame, "Frame");
		AnimationPanel.AddToGroup(&AnimationDelay, "Delay");
		AnimationPanel.AddToGroup(&AnimationXOffset, "XOffset");
		AnimationPanel.AddToGroup(&AnimationYOffset, "YOffset");
		AnimationPanel.AddToGroup(&AnimationTimeScale, "Time Scale");
		AnimationPanel.AddToGroup(&AnimationOnionSkin, "Onion Skin");

		// Draw BG
		BGBatch->Begin();
		{
			BGBatch->Add(
						EM::Vec4(-SCREENWIDTH / 2.0f, -SCREENHEIGHT / 2.0f, SCREENWIDTH, SCREENHEIGHT),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/bg_cross.png", GL_LINEAR).id,
						EG::SetOpacity(EG::RGBA16_White(), 0.1f)
					);
			BGBatch->End();
		}
		
		return true;	
	}

	bool Update()
	{
		// Check for quit condition
		IsRunning = ProcessInput(Input);

		// Update input
		Input->Update();

		// Keep track of animation delays
		if (PlayButton.State) t += TimeIncrement * TimeScale;

		// Update cameras
		Camera.Update();
		HUDCamera.Update();

		// Set up AABB of Scene Animation
		EGUI::AnimationElement::AABBSetup(&SceneAnimation);

		// Update AnimationPanel
		AnimationPanel.Update();

		if (PlayButton.State)
		{
			auto SA = SceneAnimation.CurrentAnimation;
			AnimationFrame.Set(SceneAnimation.CurrentIndex);
			AnimationDelay.Set(SA->Frames.at(SceneAnimation.CurrentIndex).Delay);
			AnimationXOffset.Set(SA->Frames.at(SceneAnimation.CurrentIndex).Offsets.x);
			AnimationYOffset.Set(SA->Frames.at(SceneAnimation.CurrentIndex).Offsets.y);
		}


		return IsRunning;
	}		

	bool Draw()
	{
		// Set up necessary matricies
    	auto Model 		= EM::Mat4::Identity();	
    	auto View 		= HUDCamera.GetCameraMatrix();
    	auto Projection = EM::Mat4::Identity();

		// Basic shader for UI
		BasicShader->Use();
		{
			BasicShader->SetUniformMat4("model", Model);
			BasicShader->SetUniformMat4("projection", Projection);
			BasicShader->SetUniformMat4("view", View);

			// Draw BG
			BGBatch->RenderBatch();

			// Reset camera to HUD
			View = Camera.GetCameraMatrix();
			BasicShader->SetUniformMat4("view", View);

			SceneBatch->Begin();
			{
				// Draw AABB of current frame
				auto CurrentAnimation = SceneAnimation.CurrentAnimation;
				auto CurrentIndex = SceneAnimation.CurrentIndex;
				auto Frame = &CurrentAnimation->Frames.at(CurrentIndex);
				auto TotalFrames = CurrentAnimation->TotalFrames;
				auto& Position = SceneAnimation.Position;

				if (SceneAnimation.HoverState == HoveredState::ON_HOVER)
				{
					auto AABB_SA = &SceneAnimation.AABB;

					// Draw border around animation
					EG::DrawRectBorder 	(
											SceneBatch,
											EM::Vec4(AABB_SA->Min, AABB_SA->Max - AABB_SA->Min),
											1.0f, 
											EG::RGBA16_Red()	
										);
				}

				if (t >= Frame->Delay)
				{
					CurrentIndex = (CurrentIndex + 1) % TotalFrames;
					SceneAnimation.CurrentIndex = CurrentIndex;
					Frame = &CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);
					t = 0.0f;
				}

				// Check for onion skin being on
				if (AnimationOnionSkin.State)
				{
					auto PreviousIndex = 0;

					// Draw the scene previous and after
					if (CurrentIndex > 0)
					{
						PreviousIndex = CurrentIndex - 1;
					}	
					else
					{
						PreviousIndex = TotalFrames - 1;
					}

					auto NextFrame = &CurrentAnimation->Frames.at((SceneAnimation.CurrentIndex + 1) % TotalFrames);
					auto PreviousFrame = &CurrentAnimation->Frames.at(PreviousIndex);

					DrawFrame(*PreviousFrame, Position, SceneBatch, EG::SetOpacity(EG::RGBA16_Blue(), 0.3f));
					DrawFrame(*NextFrame, Position, SceneBatch, EG::SetOpacity(EG::RGBA16_Red(), 0.3f));
				}

				// Draw Scene animation
				DrawFrame(*Frame, Position,	SceneBatch);

			}
			SceneBatch->End();
			SceneBatch->RenderBatch();

		}
		BasicShader->Unuse();

		TextShader->Use();
		{
			View = HUDCamera.GetCameraMatrix();

			TextShader->SetUniformMat4("model", Model);
			TextShader->SetUniformMat4("projection", Projection);
			TextShader->SetUniformMat4("view", View);

			// glEnable(GL_SCISSOR_TEST);
			UIBatch->Begin(EG::GlyphSortType::FRONT_TO_BACK);
			{
				// Draw AnimationPanel
				AnimationPanel.Draw(UIBatch);
			}
			UIBatch->End();
			UIBatch->RenderBatch();
		}
		TextShader->Unuse();

		return true;
	}

	bool ProcessInput(EI::InputManager* Input)
	{
		auto Camera = CameraManager::GetCamera("SceneCamera");
		static EM::Vec2 MouseFrameOffset(0.0f);

	    SDL_Event event;
	    while (SDL_PollEvent(&event)) 
	    {
	        switch (event.type) 
	        {
	            case SDL_QUIT:
	                return false;
	                break;
				case SDL_KEYUP:
					Input->ReleaseKey(event.key.keysym.sym); 
					break;
				case SDL_KEYDOWN:
					Input->PressKey(event.key.keysym.sym);
					break;
				case SDL_MOUSEBUTTONDOWN:
					Input->PressKey(event.button.button);
					break;
				case SDL_MOUSEBUTTONUP:
					Input->ReleaseKey(event.button.button);
					break;
				case SDL_MOUSEMOTION:
					Input->SetMouseCoords((float)event.motion.x, (float)event.motion.y);
					break;
				case SDL_MOUSEWHEEL:
					Camera->SetScale(Camera->GetScale() + (event.wheel.y) * 0.05f);
					if (Camera->GetScale() < 0.1f) Camera->SetScale(0.1f);
				default:
					break;
			}
	    }

	    if (Input->IsKeyPressed(SDLK_ESCAPE))
	    {
	    	return false;
	    }


		// Just use group for now to see if shit gon' get cray cray
		auto G = GUIManager::Get<GUIGroup>("AnimationPanel");
		static GUIElementBase* E = nullptr;
		static GUIElementBase* MouseFocus = nullptr;
		static bool PrintedDebugNoChild = false;

		// If Mouse Focus
		bool ProcessMouse = false;
		if (MouseFocus)
		{
			// Check if mouse focus is done processing itself
			if (MouseFocus->Type == GUIType::SCENE_ANIMATION)
			{
				ProcessMouse = MouseFocus->ProcessInput(Input, CameraManager::GetCamera("SceneCamera"));
		
				// If done, then return from function
				if (ProcessMouse && !Input->IsKeyDown(SDL_BUTTON_LEFT)) 
				{
					std::cout << "Losing focus after processing complete..." << std::endl;
					MouseFocus->lose_focus.emit();
					MouseFocus = nullptr;
					return true;
				}
			}
			else
			{
				ProcessMouse = MouseFocus->ProcessInput(Input, CameraManager::GetCamera("HUDCamera"));
	
				// If done, then return from function
				if (ProcessMouse) 
				{
					std::cout << "Losing focus after processing complete..." << std::endl;
					MouseFocus->lose_focus.emit();
					MouseFocus = nullptr;
					return true;
				}
			}

		}

		// Get Mouse Coords
		auto MouseCoords = Input->GetMouseCoords();
		CameraManager::GetCamera("HUDCamera")->ConvertScreenToWorld(MouseCoords);

		bool ChildFound = false;
		for (auto C : G->Children)
		{
			auto AABB = &C->AABB;

			C->check_children.emit(Input, CameraManager::GetCamera("HUDCamera"));

			if (EP::AABBvsPoint(AABB, MouseCoords))
			{
				// Set to true
				ChildFound = true;

				// Set debug to false
				PrintedDebugNoChild = false;

				// Set on hover
				C->on_hover.emit();

				// Set the hovered element
				G->HoveredElement = C;

				// If clicked while hovering
				if (Input->IsKeyPressed(SDL_BUTTON_LEFT))
				{
					C->on_click.emit();

					if (MouseFocus == nullptr) 
					{
						std::cout << "Gaining Focus: " << C->Name << std::endl;
						MouseFocus = C;
					}

					// If not mouse focus
					else if (MouseFocus != C && C->Depth >= MouseFocus->Depth && !ProcessMouse) 
					{
						std::cout << "Switching Focus: " << MouseFocus->Name << " to " << C->Name << std::endl;

						// Lose focus
						MouseFocus->lose_focus.emit();
	
						// Set focus	
						MouseFocus = C;
					}

				}
			}

			else
			{
				C->off_hover.emit();
			}
		}

		// See whether or not scene animation is covered
		if (!ChildFound)
		{
			// Get SceneAnimation
			auto SceneMousePos = Input->GetMouseCoords();
			CameraManager::GetCamera("SceneCamera")->ConvertScreenToWorld(SceneMousePos);
			auto AABB_SA = &SceneAnimation.AABB;

			// Check whether mouse is over scene animation
			auto MouseOverAnimation = EP::AABBvsPoint(AABB_SA, SceneMousePos);
			if (MouseOverAnimation)
			{
				if (SceneAnimation.HoverState == HoveredState::OFF_HOVER)
				{
					std::cout << "Entering Hover..." << std::endl;

					// Emit on hover action
					SceneAnimation.on_hover.emit();

					ChildFound = true;
				}
			}

			// If mouse was hovering nad has now left
			else if (SceneAnimation.HoverState == HoveredState::ON_HOVER)
			{
				std::cout << "Exiting Hover..." << std::endl;

				// Emit off hover action
				SceneAnimation.off_hover.emit();

				ChildFound = false;
			}

			if (MouseOverAnimation && Input->IsKeyDown(SDL_BUTTON_LEFT))
			{
				auto X = SceneMousePos.x;
				auto Y = SceneMousePos.y;

				// Turn off the play button
				if (PlayButton.State) PlayButton.on_click.emit();

				// Set mouse focus
	    		MouseFocus = &SceneAnimation;
			}
		}


		// If no children are found
		if (!ChildFound)
		{
			if (!PrintedDebugNoChild) 
			{
				std::cout << "No child..." << std::endl;
				PrintedDebugNoChild = true;
			}

			// No hovered element
			G->HoveredElement = nullptr;

			if (Input->IsKeyPressed(SDL_BUTTON_LEFT))
			{
				if (MouseFocus) 
				{
					std::cout << "Losing all focus..." << std::endl;

					MouseFocus->lose_focus.emit();
					MouseFocus = nullptr;	
					PrintedDebugNoChild = false;	
				}
			}

		}

		if (!MouseFocus)
		{

	
			if (Input->IsKeyPressed(SDLK_SPACE))
			{
				PlayButton.on_click.emit();
			}
		}



		return true;
	}


	/*
	bool ProcessInput(EI::InputManager* Input)
	{
		static bool WasHovered = false;
		static EM::Vec2 MouseFrameOffset(0.0f);
		unsigned int CurrentKey = 0;
		static std::string str = "";
		char CurrentChar = 0;

		if (KeyboardFocus)
		{
			SDL_StartTextInput();
		}
		else 
		{
			SDL_StopTextInput();
			CurrentChar = 0;
		}

	    SDL_Event event;
	    while (SDL_PollEvent(&event)) {
	        switch (event.type) {
	            case SDL_QUIT:
	                return false;
	                break;
				case SDL_KEYUP:
					Input->ReleaseKey(event.key.keysym.sym); 
					CurrentKey = 0;
					break;
				case SDL_KEYDOWN:
					Input->PressKey(event.key.keysym.sym);
					CurrentKey = event.key.keysym.sym;
					break;
				case SDL_MOUSEBUTTONDOWN:
					Input->PressKey(event.button.button);
					break;
				case SDL_MOUSEBUTTONUP:
					Input->ReleaseKey(event.button.button);
					break;
				case SDL_MOUSEMOTION:
					Input->SetMouseCoords((float)event.motion.x, (float)event.motion.y);
					break;
				case SDL_MOUSEWHEEL:
					Camera->SetScale(Camera->GetScale() + (event.wheel.y) * 0.05f);
					if (Camera->GetScale() < 0.1f) Camera->SetScale(0.1f);
				case SDL_TEXTINPUT:
					str = event.text.text;
					CurrentChar = event.text.text[0];
					std::cout << str << std::endl;
				default:
					break;
			}
	    }

		// Get Mouse Position
		auto MousePos = Input->GetMouseCoords();
		CameraManager::GetCamera("HUDCamera")->ConvertScreenToWorld(MousePos);

		// Get play button
		auto PlayButton = static_cast<GUIButton*>(GUIManager::Get("PlayButton"));
		auto AABB_PB = &PlayButton->AABB;

		// Check whether the mouse is hovered over the play button
		auto MouseOverButton = EP::AABBvsPoint(AABB_PB, MousePos);

		if (MouseOverButton)
		{
			if (PlayButton->HoverState == HoveredState::OFF_HOVER)
			{
				std::cout << "Entering Hover..." << std::endl;

				// Emit on hover action
				PlayButton->on_hover.emit();
			}
		}

		// If the mouse was hovering and has now left
		else if (PlayButton->HoverState == HoveredState::ON_HOVER)
		{
			std::cout << "Exiting Hover..." << std::endl;

			// Emit off hover action
			PlayButton->off_hover.emit();
		}

		// Get Toggle Onion skin button
		auto ToggleOnionSkin = static_cast<GUIRadialButton*>(GUIManager::Get("ToggleOnionSkin"));
		auto AABB_OS = &ToggleOnionSkin->AABB;

		// Check whether mouse button over toggle onion skin button
		auto MouseOverOnionSkin = EP::AABBvsPoint(AABB_OS, MousePos);

		if (MouseOverOnionSkin)
		{
			if (ToggleOnionSkin->HoverState == HoveredState::OFF_HOVER)
			{
				std::cout << "Hovering onion skin" << std::endl;

				// Emit hover action
				ToggleOnionSkin->on_hover.emit();
			}
		}
		else if (ToggleOnionSkin->HoverState == HoveredState::ON_HOVER)
		{
			std::cout << "Exiting Hover..." << std::endl;

			// Emit off hover action
			ToggleOnionSkin->off_hover.emit();
		}

		// Get animation selection button
		auto AnimationSelection = static_cast<GUIDropDownButton*>(GUIManager::Get("AnimationSelection"));
		auto AABB_AS = &AnimationSelection->AABB;

		// Check whether mouse button over animation selection button
		auto MouseOverAnimationSelection = EP::AABBvsPoint(AABB_AS, MousePos);

		if (MouseOverAnimationSelection)
		{
			if (AnimationSelection->HoverState == HoveredState::OFF_HOVER)
			{
				std::cout << "Entering Hover..." << std::endl;

				// Emit on hover action
				AnimationSelection->on_hover.emit();
			}
		}
		else if (AnimationSelection->HoverState == HoveredState::ON_HOVER)
		{
			std::cout << "Exiting Hover..." << std::endl;

			// Emit off hover action
			AnimationSelection->off_hover.emit();
		}

		// Get InputText
		auto InputText = static_cast<GUITextBox*>(GUIManager::Get("InputText"));
		auto AABB_IT = &InputText->AABB;

		// Check whether mouse is over the input text
		auto MouseOverText = EP::AABBvsPoint(AABB_IT, MousePos);

		if (MouseOverText)
		{
			if (InputText->HoverState == HoveredState::OFF_HOVER)
			{
				std::cout << "Entering Hover..." << std::endl;

				// Emit on_hover action
				InputText->on_hover.emit();
			}
		}

		// If the mouse was hovering and has now left
		else if (InputText->HoverState == HoveredState::ON_HOVER)
		{
			std::cout << "Exiting Hover..." << std::endl;

			// Emit off hover action
			InputText->off_hover.emit();
		}

		// Check list components for being hovered over
		static EGUI::GUITextButton* ListElement = nullptr;
		if (AnimationSelection->State == ButtonState::ACTIVE)
		{
			bool Found = false;
			for (auto i = 0; i < AnimationSelection->List.size(); ++i)
			{
				auto e = AnimationSelection->List.at(i);

				if (EP::AABBvsPoint(&e->AABB, MousePos))
				{
					// If previously assigned but now not the same, then emit off hover
					if (ListElement && ListElement != e)
					{
						if (ListElement->HoverState == HoveredState::ON_HOVER) ListElement->off_hover.emit();
					}

					// Assign list element
					ListElement = e;

					// If not hoverstate active, then emit
					if (ListElement->HoverState == HoveredState::OFF_HOVER) 
					{
						ListElement = e;
						e->on_hover.emit();
					}

					// Found, so break out of loop
					Found = true;
					break;
				}
			}
			// Nothing was selected but previous list element assigned
			if (!Found && ListElement)
			{
				ListElement->off_hover.emit();
				ListElement = nullptr;
			}
		}
		// If there was something, need to get rid of it
		else if (ListElement)
		{
			ListElement->off_hover.emit();
			ListElement = nullptr;
		}


		// Get SceneAnimation
		auto SceneAnimation = static_cast<GUIAnimationElement*>(GUIManager::Get("SceneAnimation"));
		auto SceneMousePos = Input->GetMouseCoords();
		CameraManager::GetCamera("SceneCamera")->ConvertScreenToWorld(SceneMousePos);
		auto AABB_SA = &SceneAnimation->AABB;

		// Check whether mouse is over scene animation
		auto MouseOverAnimation = EP::AABBvsPoint(AABB_SA, SceneMousePos);
		if (MouseOverAnimation)
		{
			if (SceneAnimation->HoverState == HoveredState::OFF_HOVER)
			{
				std::cout << "Entering Hover..." << std::endl;

				// Emit on hover action
				SceneAnimation->on_hover.emit();
			}
		}

		// If mouse was hovering nad has now left
		else if (SceneAnimation->HoverState == HoveredState::ON_HOVER)
		{
			std::cout << "Exiting Hover..." << std::endl;

			// Emit off hover action
				SceneAnimation->off_hover.emit();
		}


	    // Basic check for click
	    // These events need to be captured and passed to the GUI manager as signals
	    if (Input->IsKeyPressed(SDL_BUTTON_LEFT))
	    {
	    	auto X = MousePos.x;
	    	auto Y = MousePos.y;

	    	std::cout << "Mouse Pos: " << MousePos << std::endl;

	    	// Do AABB test with PlayButton
	    	if (MouseOverButton)
	    	{
	    		SelectedGUIElement = PlayButton;
	    		PlayButton->on_click.emit();
	    		MouseFocus = nullptr;
	    	}

	    	else if (MouseOverOnionSkin)
	    	{
	    		ToggleOnionSkin->on_click.emit();
	    		if (AnimationSelection->State) AnimationSelection->on_click.emit();
	    	}

	    	// Do AABB test with AnimationSelection List Element
	    	else if (ListElement)
	    	{
	    		ListElement->on_click.emit(ListElement);
	    		ListElement->off_hover.emit();
	    		ListElement = nullptr;
	    		MouseFocus = nullptr;
	    	}

	    	else if (MouseOverAnimationSelection)
	    	{
	    		SelectedGUIElement = AnimationSelection;
	    		AnimationSelection->on_click.emit();
	    		MouseFocus = AnimationSelection;
	    	}

	    	else if (MouseOverText)
	    	{
	    		SelectedGUIElement = InputText;
	    		KeyboardFocus = InputText;
	    		MouseFocus = nullptr; 			// The way to do this eventually is set all of these focuses here to this element but define whether or not it can move
	    		InputText->on_click.emit(MousePos.x);
	    	}

	    	else if (MouseOverAnimation)
	    	{
				// Turn off the play button
				if (PlayButton->State == ButtonState::ACTIVE) PlayButton->on_click.emit();

	    		SelectedGUIElement = SceneAnimation;
	    		MouseFocus = SceneAnimation;
	    		MouseFrameOffset = EM::Vec2(SceneMousePos.x - SceneAnimation->AABB.Min.x, SceneMousePos.y - SceneAnimation->AABB.Min.y);
	    	}

	    	else
	    	{
	    		// Deselect current thing if was selected
	    		if (MouseFocus) 
    			{
		    		if (MouseFocus == AnimationSelection && AnimationSelection->State) MouseFocus->on_click.emit();
		    	}

	    		// This is incredibly not thought out at all...
	    		SelectedGUIElement = nullptr;
	    		MouseFocus = nullptr;
	    		KeyboardFocus = nullptr;
	    	}
	    }

	    // NOTE(John): Again, these input manager states will be hot loaded in, so this will be cleaned up eventaully...
		if (MouseFocus)
		{
			if (Input->IsKeyDown(SDL_BUTTON_LEFT))
			{
				auto X = SceneMousePos.x;
				auto Y = SceneMousePos.y;

				if (MouseFocus->Type == GUIType::SCENE_ANIMATION)
				{
					auto Anim = static_cast<GUIAnimationElement*>(MouseFocus);
					auto CurrentAnimation = Anim->CurrentAnimation;	

					// Find bottom corner of current frame
					auto BottomCorner = Anim->AABB.Min;

					// Update offsets
					CurrentAnimation->Frames.at(Anim->CurrentIndex).Offsets = EM::Vec2(X - MouseFrameOffset.x, Y - MouseFrameOffset.y);
				}
			}
		}

		if (Input->IsKeyPressed(SDLK_ESCAPE))
		{
			return false;	
		}

		if (KeyboardFocus && CurrentKey != 0)
		{
			// Check for modifiers first
			if (!IsModifier(CurrentKey))
			{
			if (CurrentKey == SDLK_BACKSPACE) InputText->on_backspace.emit();
				else if (CurrentKey == SDLK_LEFT)
				{
					if (InputText->CursorIndex > 0) InputText->CursorIndex--;
				}
				else if (CurrentKey == SDLK_RIGHT)
				{
					if (InputText->CursorIndex < InputText->Text.length()) InputText->CursorIndex++;
				}
				else InputText->on_keyboard.emit(str);
			}
		}


		else if (KeyboardFocus == nullptr)
		{
			// Switch off input text
			InputText->KeyboardInFocus = false;

			if (Input->IsKeyDown(SDLK_e))
			{
				Camera->SetScale(Camera->GetScale() + 0.05f);
			}
			if (Input->IsKeyDown(SDLK_q))
			{
				auto S = Camera->GetScale();
				if (S > 0.1f) Camera->SetScale(S - 0.05f);
			}
			if (Input->IsKeyPressed(SDLK_SPACE))
			{
				// Get button from button manager
				auto PlayButton = static_cast<GUIButton*>(GUIManager::Get("PlayButton"));

				// Press play
				PlayButton->on_click.emit();
			}
			if (Input->IsKeyDown(SDLK_RIGHT))
			{
				// Get button from button manager
				auto OffsetRight = static_cast<GUIButton*>(GUIManager::Get("OffsetRight"));

				// Press next frame
				OffsetRight->on_click.emit();	
			}
			if (Input->IsKeyDown(SDLK_LEFT))
			{
				// Get button from button manager
				auto OffsetLeft = static_cast<GUIButton*>(GUIManager::Get("OffsetLeft"));

				// Press next frame
				OffsetLeft->on_click.emit();
			}
			if (Input->IsKeyDown(SDLK_UP))
			{
				// Get button from button manager
				auto OffsetUp = static_cast<GUIButton*>(GUIManager::Get("OffsetUp"));

				// Press offset up
				OffsetUp->on_click.emit();
			}
			if (Input->IsKeyDown(SDLK_DOWN))
			{
				// Get button from button manager
				auto OffsetDown = static_cast<GUIButton*>(GUIManager::Get("OffsetDown"));

				// Press offset Down
				if (OffsetDown)
				{
					OffsetDown->on_click.emit();
				}
			}
			if (Input->IsKeyPressed(SDLK_m))
			{
				// Get button from button manager
				auto NextFrame = static_cast<GUIButton*>(GUIManager::Get("NextFrame"));

				// Press offset Down
				if (NextFrame)
				{
					NextFrame->on_click.emit();
				}
			}
			if (Input->IsKeyPressed(SDLK_n))
			{
				// Get button from button manager
				auto PreviousFrame = static_cast<GUIButton*>(GUIManager::Get("PreviousFrame"));

				// Press offset Down
				if (PreviousFrame)
				{
					PreviousFrame->on_click.emit();
				}

			}
			if (Input->IsKeyPressed(SDLK_LEFTBRACKET))
			{
				// Get button from button manager
				auto DelayDown = static_cast<GUIButton*>(GUIManager::Get("DelayDown"));

				// Emit
				if (DelayDown)
				{
					DelayDown->on_click.emit();	
				}	
			}
			if (Input->IsKeyPressed(SDLK_RIGHTBRACKET))
			{
				// Get button from button manager
				auto DelayUp = static_cast<GUIButton*>(GUIManager::Get("DelayUp"));

				// Emit
				if (DelayUp)
				{
					DelayUp->on_click.emit();
				}
			}
			if (Input->IsKeyPressed(SDLK_o))
			{
				// Get button from button manager
				auto ToggleOnionSkin = static_cast<GUIButton*>(GUIManager::Get("ToggleOnionSkin"));

				// Emit
				if (ToggleOnionSkin)
				{
					ToggleOnionSkin->on_click.emit();
				}
			}

			if (Input->IsKeyDown(SDLK_t))
			{
				if (TimeScale > 0.0f) TimeScale -= 0.01f;
			}

			if (Input->IsKeyDown(SDLK_y))
			{
				if (TimeScale < 1.0f) TimeScale += 0.01f;
			}
		}

		return true;
	}
	*/

	void CalculateAABBWithParent(EP::AABB* A, GUIButton* Button)
	{
		auto Parent = Button->Parent;
		auto PPos = &Parent->Position;

		// Set up PlayButton AABB
		Button->AABB.Min =  EM::Vec2(Button->Position.x + PPos->x + Button->Frames.at(ButtonState::INACTIVE).Offsets.x * Button->Frames.at(ButtonState::INACTIVE).ScalingFactor,
										Button->Position.y + PPos->y + Button->Frames.at(ButtonState::INACTIVE).Offsets.y * Button->Frames.at(ButtonState::INACTIVE).ScalingFactor); 
		Button->AABB.Max = Button->AABB.Min + EM::Vec2(Button->Frames.at(ButtonState::INACTIVE).SourceSize.x * Button->Frames.at(ButtonState::INACTIVE).ScalingFactor, 
											     Button->Frames.at(ButtonState::INACTIVE).SourceSize.y * Button->Frames.at(ButtonState::INACTIVE).ScalingFactor); 
	}

	bool IsModifier(unsigned int Key)
	{
		if (Key == SDLK_LSHIFT || 
			Key == SDLK_RSHIFT || 
			Key == SDLK_LCTRL  ||
			Key == SDLK_RCTRL  ||
			Key == SDLK_CAPSLOCK)
		return true;

		else return false; 
	}

	bool HasChildren(GUIElementBase* E)
	{
		if (E->Type == GUIType::DROP_DOWN_BUTTON ||
			E->Type == GUIType::GROUP)
		return true;

		return false;
	}

}}
