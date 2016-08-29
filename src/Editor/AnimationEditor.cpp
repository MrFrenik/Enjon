/*-- Enjon includes --*/
#include "Editor/AnimationEditor.h"
#include "IO/InputManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/FontManager.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/CursorManager.h"
#include "GUI/GUIAnimationElement.h"
#include "GUI/GUIGroup.h"
#include "Physics/AABB.h"
#include "Utils/Errors.h"
#include "Defines.h"

/*-- IsoARPG includes --*/
#include "EnjonAnimation.h"
#include "AnimationManager.h"
#include "AnimManager.h"
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
	GUIButton 					SaveAnimationToFile;
	GUIButton 					AnimationPanelIcon;
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

	float TimeScale = 1.0f;
	float TimeIncrement = 0.15f;
	float t = 0.0f;

	float CameraScaleVelocityGoal = 0.0f;
	float CameraScaleVelocity = 0.0f;

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

		PlayButton.Dimensions = EM::Vec2(16.0f, 10.0f);
		PlayButton.AABB.Min = PlayButton.Position;
		PlayButton.AABB.Max = PlayButton.AABB.Min + PlayButton.Dimensions;
		PlayButton.Text = "J";
		PlayButton.TextFont = EG::FontManager::GetFont("Arrows7");
		PlayButton.State = ButtonState::INACTIVE;


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
			SaveAnimationToFile.Text = "1";
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
			SaveAnimationToFile.Text = "1";
		});

		AnimationYOffset.lose_focus.connect([&]()
		{
			SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex).Offsets.y = AnimationYOffset.Value.get();
			SaveAnimationToFile.Text = "1";
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
			SaveAnimationToFile.Text = "1";
		});

		SceneAnimation.on_click.connect([&]()
		{
			if (!AnimationPanel.Visibility) AnimationPanelIcon.on_click.emit();
		});

		AnimationPanelIcon.on_click.connect([&]()
		{
			// Turn on animation panel
			AnimationPanel.Visibility = VisibleState::VISIBLE;

			// Turn off icon
			AnimationPanelIcon.Visibility = VisibleState::HIDDEN;
		});

		AnimationPanel.MinimizeButton.on_click.connect([&]()
		{
			// Turn on icon
			AnimationPanelIcon.Visibility = VisibleState::VISIBLE;
		});

		SaveAnimationToFile.on_click.connect([&]()
		{
			// Get file path
			auto FilePath = SceneAnimation.CurrentAnimation->FilePath.c_str();

			// Get JSON string
			auto Json = EU::read_file_sstream(FilePath);

		   	// Parese and serialize JSON
		   	json j_complete = json::parse(Json);

		   	// Get name of aniamtion
		   	auto Name = SceneAnimation.CurrentAnimation->Name;

		   	// Set info
		   	auto i = 0;
		   	for (auto& frame : SceneAnimation.CurrentAnimation->Frames)
		   	{
		   		// Delay
		   		j_complete.at(Name).at("delays").at(i) = frame.Delay;

		   		// Xoffset
		   		j_complete.at(Name).at("xoffsets").at(i) = frame.Offsets.x;

		   		// Yoffset
		   		j_complete.at(Name).at("yoffsets").at(i) = frame.Offsets.y;

		   		// Increment index
		   		i++;
		   	}

		   	// Save to file
		   	EU::save_to_json(FilePath, j_complete, 4);

		   	// Set text to check mark
		   	SaveAnimationToFile.Text = "u";
		   	SaveAnimationToFile.TextColor = EG::RGBA16_LimeGreen();
		});

		SaveAnimationToFile.on_hover.connect([&]()
		{
			if (SaveAnimationToFile.Text.compare("u") == 0) SaveAnimationToFile.TextColor = EG::RGBA16_LimeGreen();
			else 											SaveAnimationToFile.TextColor = EG::RGBA16_MidGrey(); 
		});

		SaveAnimationToFile.off_hover.connect([&]()
		{
			if (SaveAnimationToFile.Text.compare("u") == 0) SaveAnimationToFile.TextColor = EG::SetOpacity(EG::RGBA16_LimeGreen(), 0.4f);
			else 											SaveAnimationToFile.TextColor = EG::SetOpacity(EG::RGBA16_MidGrey(), 0.4f); 
		});

		// Add to GUIManager
		GUIManager::Add("PlayButton", &PlayButton);
		GUIManager::Add("AnimationPanel", &AnimationPanel);
		GUIManager::Add("SceneGroup", &SceneGroup);

		// Set up Animation Icon button
		AnimationPanelIcon.Name = "Animation Icon";
		AnimationPanelIcon.Position = EM::Vec2(-SCREENWIDTH / 2.0f + 20.0f, -SCREENHEIGHT / 2.0f + 20.0f);
		AnimationPanelIcon.Dimensions = EM::Vec2(12.0f, 12.0f);
		AnimationPanelIcon.AABB.Min = AnimationPanelIcon.Position;
		AnimationPanelIcon.AABB.Max = AnimationPanelIcon.AABB.Min + AnimationPanelIcon.Dimensions;
		AnimationPanelIcon.Text = "x";
		AnimationPanelIcon.TextFont = EG::FontManager::GetFont("Arrows7");
		AnimationPanelIcon.Visibility = VisibleState::HIDDEN;

		// Set up AnimationPanel Group
		AnimationPanel.Name = "Animation Editor";
		AnimationPanel.Position = EM::Vec2(-SCREENWIDTH / 2.0f + 40.0f, 20.0f);														// Set position of group

		AnimationDelay.Step = 0.1f;
		AnimationDelay.MinValue = 0.000001f;
		AnimationDelay.Set(SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex).Delay); 

		AnimationFrame.MaxValue = SceneAnimation.CurrentAnimation->Frames.size() - 1;
		AnimationFrame.MinValue = 0;
		AnimationFrame.Set(SceneAnimation.CurrentIndex);
		AnimationFrame.LoopValues = true;
		AnimationFrame.ValueUp.Text = ">";
		AnimationFrame.ValueDown.Text = "<";
		AnimationFrame.FontScale = 0.8f;
		AnimationFrame.YOffset = 3.0f;

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

		SaveAnimationToFile.Name = "Save";
		SaveAnimationToFile.Text = "u";
		SaveAnimationToFile.TextColor = EG::RGBA16_LimeGreen();
		SaveAnimationToFile.TextFont = EG::FontManager::GetFont("CutOut");
		SaveAnimationToFile.Visibility = VisibleState::VISIBLE;
		SaveAnimationToFile.Clickability = ClickState::NOT_CLICKABLE;

		AnimationPanel.AddToGroup(&AnimationSelection, "Animation");
		AnimationPanel.AddToGroup(&AnimationFrame, "Frame");
		AnimationPanel.AddToGroup(&AnimationDelay, "Delay");
		AnimationPanel.AddToGroup(&AnimationXOffset, "XOffset");
		AnimationPanel.AddToGroup(&AnimationYOffset, "YOffset");
		AnimationPanel.AddToGroup(&AnimationTimeScale, "Time Scale");
		AnimationPanel.AddToGroup(&AnimationOnionSkin, "Onion Skin");
		AnimationPanel.AddToGroup(&PlayButton, "Controls");
		AnimationPanel.AddToGroup(&SaveAnimationToFile, "Save");

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

		// Zoom in with camera velocity
		float Max = 0.2f;
		CameraScaleVelocity = Enjon::Math::Lerp(CameraScaleVelocityGoal, CameraScaleVelocity, 0.0065f);
		if (std::fabs(CameraScaleVelocity) > Max) 
		{
			if (CameraScaleVelocity < 0.0f)  CameraScaleVelocity = -Max;
			else CameraScaleVelocity = Max;
		}

		// Set scale of camera
		Camera.SetScale(Camera.GetScale() + CameraScaleVelocity);
		if (Camera.GetScale() < 0.1f) Camera.SetScale(0.1f);

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
					auto SecondPreviousIndex = 0;

					// Draw the scene previous and after
					if (CurrentIndex > 0)
					{
						PreviousIndex = CurrentIndex - 1;
					}	
					else
					{
						PreviousIndex = TotalFrames - 1;
					}

					if (CurrentIndex >= 2) SecondPreviousIndex = CurrentIndex - 2;

					auto NextFrame = &CurrentAnimation->Frames.at((SceneAnimation.CurrentIndex + 1) % TotalFrames);
					auto PreviousFrame = &CurrentAnimation->Frames.at(PreviousIndex);
					auto SecondPreviousFrame = &CurrentAnimation->Frames.at(SecondPreviousIndex);

					DrawFrame(*PreviousFrame, Position, SceneBatch, EG::SetOpacity(EG::RGBA16_Blue(), 0.3f));
					DrawFrame(*NextFrame, Position, SceneBatch, EG::SetOpacity(EG::RGBA16_Red(), 0.3f));

					// DrawFrame(*SecondPreviousFrame, Position, SceneBatch, EG::SetOpacity(EG::RGBA16_White(), 0.1f));
					// DrawFrame(*PreviousFrame, Position, SceneBatch, EG::SetOpacity(EG::RGBA16_White(), 0.3f));
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
				if (AnimationPanel.Visibility) AnimationPanel.Draw(UIBatch);

				// Draw AnimationIcon
				if (AnimationPanelIcon.Visibility)
				{
					// Draw border
					UIBatch->Add(
									EM::Vec4(AnimationPanelIcon.Position, AnimationPanelIcon.Dimensions),
									EM::Vec4(0, 0, 1, 1),
									EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
									AnimationPanelIcon.Color
								);

					// Draw text
					EG::Fonts::PrintText(
											AnimationPanelIcon.Position.x + EG::Fonts::GetAdvance(AnimationPanelIcon.Text[0], AnimationPanelIcon.TextFont, 1.0f) / 2.0f + 3.0f,
											AnimationPanelIcon.Position.y + EG::Fonts::GetHeight(AnimationPanelIcon.Text[0], AnimationPanelIcon.TextFont, 1.0f) / 2.0f,
											1.0f,
											AnimationPanelIcon.Text,
											AnimationPanelIcon.TextFont,
											*UIBatch,
											EG::SetOpacity(EG::RGBA16_MidGrey(), 0.6f), 
											EG::Fonts::TextStyle::SHADOW,
											EM::ToRadians(180.0f)
										);
				}

				if (AnimationPanel.Visibility)
				{
					// Draw Playbutton
					UIBatch->Add(
									EM::Vec4(PlayButton.Position, PlayButton.Dimensions), 
									EM::Vec4(0, 0, 1, 1), 
									EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
									PlayButton.Color,
									0.0f,
									EG::SpriteBatch::DrawOptions::BORDER | EG::SpriteBatch::DrawOptions::SHADOW,
									EG::SetOpacity(EG::RGBA16_MidGrey(), 0.3f),
									1.0f,
									EM::Vec2(2.0f, 2.0f)
								);
		
					// Draw text
					if (!PlayButton.State)
					{
						EG::Fonts::PrintText(
												PlayButton.Position.x + EG::Fonts::GetHeight(PlayButton.Text[0], PlayButton.TextFont, 1.0f) / 2.0f + 2.0f,
												PlayButton.Position.y + EG::Fonts::GetAdvance(PlayButton.Text[0], PlayButton.TextFont, 1.0f) / 2.0f + 1.0f,
												1.0f,
												PlayButton.Text,
												PlayButton.TextFont,
												*UIBatch,
												EG::SetOpacity(EG::RGBA16_MidGrey(), 0.6f), 
												EG::Fonts::TextStyle::SHADOW,
												EM::ToRadians(-90.0f)
											);
					}
					else
					{
						// Draw Playbutton
						auto Offset = EM::Vec2(PlayButton.Dimensions.x / 4.0f, PlayButton.Dimensions.y / 4.0f);
						UIBatch->Add(
										EM::Vec4(PlayButton.Position + Offset, PlayButton.Dimensions.x / 2.0f, PlayButton.Dimensions.y / 1.75f), 
										EM::Vec4(0, 0, 1, 1), 
										EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
										EG::SetOpacity(EG::RGBA16_MidGrey(), 0.6f),
										0.0f,
										EG::SpriteBatch::DrawOptions::SHADOW,
										EG::RGBA16_White(),
										1.0f,
										EM::Vec2(1.0f, 1.0f)
									);
					}

					EG::Fonts::PrintText(
											SaveAnimationToFile.Position.x,
											SaveAnimationToFile.Position.y + 2.0f,
											1.0f,
											SaveAnimationToFile.Text,
											SaveAnimationToFile.TextFont,
											*UIBatch,
											SaveAnimationToFile.TextColor
										);

				}
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
					CameraScaleVelocity += event.wheel.y * 0.05f;
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
				else return true;	
			}

			else if (MouseFocus->Type == GUIType::GROUP)
			{
				ProcessMouse = MouseFocus->ProcessInput(Input, CameraManager::GetCamera("HUDCamera"));
		
				// If done, then return from function
				if (ProcessMouse && !Input->IsKeyDown(SDL_BUTTON_LEFT)) 
				{
					std::cout << "Losing focus after processing complete..." << std::endl;
					MouseFocus->lose_focus.emit();
					MouseFocus = nullptr;
					return true;
				}
				else return true;
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

		// Check for minimize button in 
		auto MouseOverMinimizeButton = EP::AABBvsPoint(&G->MinimizeButton.AABB, MouseCoords);
		if (MouseOverMinimizeButton)
		{
			if (!G->MinimizeButton.HoverState) G->MinimizeButton.on_hover.emit();

			if (Input->IsKeyPressed(SDL_BUTTON_LEFT))
			{
				G->MinimizeButton.on_click.emit();
				return true;
			}
		}
		else if (G->MinimizeButton.HoverState) G->MinimizeButton.off_hover.emit();

		bool ChildFound = false;

		if (G->Visibility)
		{
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
				// Turn off the play button
				if (PlayButton.State) PlayButton.on_click.emit();

				// Set mouse focus
	    		MouseFocus = &SceneAnimation;
			}

			if (AnimationPanel.Visibility)
			{
				auto MouseOverAnimationPanel = EP::AABBvsPoint(&AnimationPanel.AABB, MouseCoords);
				if (MouseOverAnimationPanel && !AnimationPanel.MinimizeButton.HoverState)
				{
					if (!AnimationPanel.HoverState) AnimationPanel.on_hover.emit();
					ChildFound = true;
				}
				else
				{
					if (AnimationPanel.HoverState) AnimationPanel.off_hover.emit();
					ChildFound = false;
				}

				if (MouseOverAnimationPanel && Input->IsKeyDown(SDL_BUTTON_LEFT))
				{
					// Set mouse focus
		    		MouseFocus = &AnimationPanel;
				}
			}	

			if (AnimationPanelIcon.Visibility)
			{
				auto MouseOverAnimationIcon = EP::AABBvsPoint(&AnimationPanelIcon.AABB, MouseCoords);
				if (MouseOverAnimationIcon)
				{
					if (!AnimationPanelIcon.HoverState) AnimationPanelIcon.on_hover.emit();
					ChildFound = true;
				}
				else
				{
					if (AnimationPanelIcon.HoverState) AnimationPanelIcon.off_hover.emit();
					ChildFound = false;
				}

				if (MouseOverAnimationIcon && Input->IsKeyPressed(SDL_BUTTON_LEFT))
				{
					AnimationPanelIcon.on_click.emit();
				}
			}

			auto MouseOverPlayButton = EP::AABBvsPoint(&PlayButton.AABB, MouseCoords);
			if (MouseOverPlayButton)
			{
				if (!PlayButton.HoverState) PlayButton.on_hover.emit();
				ChildFound = true;
			}
			else
			{
				if (PlayButton.HoverState) PlayButton.off_hover.emit();
			}

			if (MouseOverPlayButton && Input->IsKeyPressed(SDL_BUTTON_LEFT))
			{
				PlayButton.on_click.emit();
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

			if (Input->IsKeyPressed(SDLK_m))
			{
				AnimationFrame.ValueUp.on_click.emit();
				if (PlayButton.State) PlayButton.on_click.emit();
			}

			if (Input->IsKeyPressed(SDLK_n))
			{
				AnimationFrame.ValueDown.on_click.emit();
				if (PlayButton.State) PlayButton.on_click.emit();
			}
		}



		return true;
	}
}}
