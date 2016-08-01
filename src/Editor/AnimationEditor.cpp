/*-- Enjon includes --*/
#include "Editor/AnimationEditor.h"
#include "IO/InputManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/FontManager.h"
#include "Graphics/SpriteBatch.h"
#include "GUI/GUIAnimationElement.h"
#include "Physics/AABB.h"
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

	// Something like this eventually for global gui references...
	namespace ButtonManager
	{
		std::unordered_map<std::string, GUIButton*> Buttons;

		void Add(std::string S, GUIButton* B)
		{
			Buttons[S] = B;
		}

		GUIButton* Get(const std::string S)
		{
			auto search = Buttons.find(S);
			if (search != Buttons.end())
			{
				return search->second;
			}	

			return nullptr;
		}
	};

	// This is stupid, but it's for testing...
	namespace TextBoxManager
	{
		std::unordered_map<std::string, GUITextBox*> TextBoxes;

		void Add(std::string S, GUITextBox* T)
		{
			TextBoxes[S] = T;
		}

		GUITextBox* Get(const std::string S)
		{
			auto search = TextBoxes.find(S);
			if (search != TextBoxes.end())
			{
				return search->second;
			}
			return nullptr;
		}
	};

	namespace GUIManager
	{
		std::unordered_map<std::string, GUIElementBase*> Elements;

		void Add(std::string S, GUIElementBase* E)
		{
			Elements[S] = E;
		}

		GUIElementBase* Get(const std::string S)
		{
			auto search = Elements.find(S);
			if (search != Elements.end())
			{
				return search->second;
			}

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

namespace CursorManager
{
	std::unordered_map<std::string, SDL_Cursor*> Cursors;

	void Init()
	{
		Cursors["Arrow"] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
		Cursors["IBeam"] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	}

	SDL_Cursor* Get(const std::string S)
	{
		auto search = Cursors.find(S);
		if (search != Cursors.end())
		{
			return search->second;
		}
		return nullptr;
	}

};

namespace Enjon { namespace AnimationEditor {

	using namespace GUI;

	/*-- Function Delcarations --*/
	bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera);
	void CalculateAABBWithParent(EP::AABB* A, GUIButton* Button);
	bool IsModifier(unsigned int Key);

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

	GUIButton 			PlayButton;
	GUIButton 			NextFrame;
	GUIButton 			PreviousFrame;
	GUIButton 			OffsetUp;
	GUIButton 			OffsetDown;
	GUIButton 			OffsetLeft;
	GUIButton 			OffsetRight;
	GUIButton 			DelayUp;
	GUIButton 			DelayDown;
	GUIButton			ToggleOnionSkin;
	GUIButton 			PlayerAttackOLSEButton; 		// Obviously these need to be generated procedurally...
	GUIButton 			PlayerAttackOLSWButton;
	GUITextBox 			InputText;
	GUIAnimationElement SceneAnimation;
	GUIDropDownButton 	AnimationSelection;	
	GUIGroup 			Group;
	GUIGroup 			AnimationInfoGroup;
	GUIGroup 			AnimationSelectionGroup;
	GUIGroup			PlayerAnimationGroup;	
	
	
	Atlas atlas;
	float AWidth;
	float AHeight;
	
	float SCREENWIDTH;
	float SCREENHEIGHT;

	float caret_count = 0.0f;
	bool caret_on = true;
	float TimeScale = 1.0f;
	float TimeIncrement = 0.0f;
	float t = 0.0f;

	bool IsRunning = true;
	
	EG::GLSLProgram* BasicShader 	= nullptr;
	EG::GLSLProgram* TextShader 	= nullptr;
	EG::SpriteBatch* UIBatch 		= nullptr;
	EG::SpriteBatch* SceneBatch 	= nullptr;
	EG::SpriteBatch* BGBatch 		= nullptr;

	EG::Camera2D Camera;
	EG::Camera2D HUDCamera;

	EG::ColorRGBA16 PlayButtonColor;
	EG::ColorRGBA16 InputTextColor;
	EG::ColorRGBA16 AnimationSelectionColor;
	EG::ColorRGBA16 PlayerAttackOLSEColor;
	EG::ColorRGBA16 PlayerAttackOLSWColor;

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
		
		// Set up ToggleOnionSkin
		ToggleOnionSkin.Type = GUIType::BUTTON;
		ToggleOnionSkin.State = ButtonState::INACTIVE;

		// Set up Scene Animtation
		SceneAnimation.CurrentAnimation = AnimManager::GetAnimation("Player_Attack_OH_L_SE");
		SceneAnimation.CurrentIndex = 0;
		SceneAnimation.Position = EM::Vec2(0.0f);
		SceneAnimation.State = ButtonState::INACTIVE;
		SceneAnimation.HoverState = HoveredState::OFF_HOVER;
		SceneAnimation.Type = GUIType::SCENE_ANIMATION;

		// Set up AnimationSelection's text with name of current animation
		AnimationSelection.Text = SceneAnimation.CurrentAnimation->Name;
		AnimationSelection.Type = GUIType::BUTTON;
		AnimationSelection.State = ButtonState::INACTIVE;
		AnimationSelection.HoverState = HoveredState::OFF_HOVER;
		AnimationSelection.Position = EM::Vec2 (
													HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 105.0f,
													HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - 100.0f 
												);

		// Add animations to AnimationSelection drop down button
		for (auto& a : *AnimManager::GetAnimationMap())
		{
			// Make button to be pushed back
			auto b = new GUITextButton();

			// Set up button state
			b->State = ButtonState::INACTIVE;

			// Set up hover state
			b->HoverState = HoveredState::OFF_HOVER;

			// Give name of button name of current animation
			b->Text = a.first;

			// Set up color
			b->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);

			// Set up off_hover signal
			b->on_hover.connect([&](GUIElementBase* b)
			{
				std::cout << "Entering list element..." << std::endl;
				b->HoverState = HoveredState::ON_HOVER;
				b->Color = EG::RGBA16(0.1f, 0.1f, 0.1f, 1.0f);
			});

			// Set up on_hover signal
			b->off_hover.connect([&](GUIElementBase* b)
			{
				std::cout << "Leaving list element..." << std::endl;
				b->HoverState = HoveredState::OFF_HOVER;
				b->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
			});

			// Set up on_hover signal
			b->on_click.connect([&](GUIElementBase* b)
			{
				// Set Current animation to this
				SceneAnimation.CurrentAnimation = AnimManager::GetAnimation(b->Text);

				// Set text of animation selection
				AnimationSelection.Text = b->Text;

				// Deactive animation selection
				AnimationSelection.on_click.emit();
			});

			// Push back into drop down list
			AnimationSelection.List.push_back(b);
		}


		// Calculate AnimationSelection's dimensions
		// Naive way first - get mouse position in UI space
		{
			std::string& T = AnimationSelection.Text;
			float A = 0.0f;

			std::cout << T << std::endl;

			// Get advance
			for (auto& c : T)
			{
				// Summation of all characters
				A += EG::Fonts::GetAdvance(c, EG::FontManager::GetFont("WeblySleek"), 1.0f);
			}

			float Padding = 20.0f;
			std::cout << "A: " << A << std::endl;
			AnimationSelection.Dimensions = EM::Vec2(A + Padding, 20.0f);
		}


		// Set up text box's text
		InputText.Text = std::string("");
		InputText.CursorIndex = 0;

		// Set up colors of buttons
		PlayButtonColor 		= EG::RGBA16_White();
		InputTextColor 			= EG::RGBA16(0.05f, 0.05f, 0.05f, 0.4f);
		AnimationSelectionColor = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
		PlayerAttackOLSWColor	= EG::RGBA16(0.25f, 0.25f, 0.25f, 0.3f);
		PlayerAttackOLSEColor	= EG::RGBA16(0.25f, 0.25f, 0.25f, 0.3f);


		PlayButton.Type = GUIType::BUTTON;
		InputText.Type = GUIType::TEXTBOX;

		// Set up Group
		Group.Position = EM::Vec2(0.0f, -400.0f);

		// Add PlayButton to Group
		GUI::AddToGroup(&Group, &PlayButton);
		GUI::AddToGroup(&Group, &NextFrame);
		GUI::AddToGroup(&Group, &PreviousFrame);
		GUI::AddToGroup(&Group, &InputText);

		// Set up play button image frames
		PlayButton.Frames.push_back(EA::GetImageFrame(Frames, "playbuttonup", AnimTextureDir));
		PlayButton.Frames.push_back(EA::GetImageFrame(Frames, "playbuttondown", AnimTextureDir));

		PlayButton.Frames.at(ButtonState::INACTIVE).TextureAtlas 	= atlas;
		PlayButton.Frames.at(ButtonState::ACTIVE).TextureAtlas 		= atlas;

		// Set up PlayButton offsets
		{
			auto xo = 0.0f;
			auto yo = 0.0f;
			PlayButton.Frames.at(ButtonState::INACTIVE).Offsets.x 	= xo;
			PlayButton.Frames.at(ButtonState::INACTIVE).Offsets.y 	= yo;
			PlayButton.Frames.at(ButtonState::ACTIVE).Offsets.x 	= xo;
			PlayButton.Frames.at(ButtonState::ACTIVE).Offsets.y 	= yo;
		}

		// Set up PlayButton position within the group
		PlayButton.Position = EM::Vec2(10.0f, 20.0f);

		// Set state to inactive
		PlayButton.State 		= ButtonState::INACTIVE;
		PlayButton.HoverState 	= HoveredState::OFF_HOVER;

		// Set up Scaling Factor
		PlayButton.Frames.at(ButtonState::INACTIVE).ScalingFactor 	= 1.0f;
		PlayButton.Frames.at(ButtonState::ACTIVE).ScalingFactor 	= 1.0f;

		// Set up PlayButton AABB
		PlayButton.AABB.Min =  EM::Vec2(PlayButton.Position.x + Group.Position.x + PlayButton.Frames.at(ButtonState::INACTIVE).Offsets.x * PlayButton.Frames.at(ButtonState::INACTIVE).ScalingFactor,
										PlayButton.Position.y + Group.Position.y + PlayButton.Frames.at(ButtonState::INACTIVE).Offsets.y * PlayButton.Frames.at(ButtonState::INACTIVE).ScalingFactor); 
		PlayButton.AABB.Max = PlayButton.AABB.Min + EM::Vec2(PlayButton.Frames.at(ButtonState::INACTIVE).SourceSize.x * PlayButton.Frames.at(ButtonState::INACTIVE).ScalingFactor, 
											     PlayButton.Frames.at(ButtonState::INACTIVE).SourceSize.y * PlayButton.Frames.at(ButtonState::INACTIVE).ScalingFactor);

		// Set up InputText position within the group
		InputText.Position = EM::Vec2(100.0f, 60.0f);

		// Set states to inactive
		InputText.State 		= ButtonState::INACTIVE;
		InputText.HoverState 	= HoveredState::OFF_HOVER;

		// Set up InputText AABB
		// This will be dependent on the size of the text, or it will be static, or it will be dependent on some image frame
		InputText.AABB.Min = InputText.Position + Group.Position;
		InputText.AABB.Max = InputText.AABB.Min + EM::Vec2(200.0f, 20.0f);

		// Set up AnimationSelection AABB
		// Calculate size of button
		AnimationSelection.AABB.Min = AnimationSelection.Position;
		AnimationSelection.AABB.Max = AnimationSelection.AABB.Min + AnimationSelection.Dimensions;

		// Add AnimationSelectionGroup to AnimationInfoGroup
		GUI::AddToGroup(&AnimationInfoGroup, &AnimationSelectionGroup);

		// Add AnimationSelection to group
		GUI::AddToGroup(&AnimationSelectionGroup, &AnimationSelection);

		// Calculate Group's AABB by its children's AABBs 
		Group.AABB.Min = Group.Position;
		// Figure out height
		auto GroupHeight = InputText.AABB.Max.y - Group.AABB.Min.y;
		auto GroupWidth = InputText.AABB.Max.x - Group.AABB.Min.x;
		Group.AABB.Max = Group.AABB.Min + EM::Vec2(GroupWidth, GroupHeight);

		// Set up ToggleOnionSkin's on_click signal
		ToggleOnionSkin.on_click.connect([&]()
		{
			std::cout << "Emiting onion skin..." << std::endl;

			ToggleOnionSkin.State = ToggleOnionSkin.State == ButtonState::INACTIVE ? ButtonState::ACTIVE : ButtonState::INACTIVE;
		});


		// Set up AnimationSelection's on_hover signal
		AnimationSelection.on_hover.connect([&]()
		{
			AnimationSelection.HoverState = HoveredState::ON_HOVER;
			AnimationSelectionColor = EG::RGBA16(0.1f, 0.1f, 0.1f, 1.0f);
		});

		// Set up AnimationSelection's off_hover signal
		AnimationSelection.off_hover.connect([&]()
		{
			AnimationSelection.HoverState = HoveredState::OFF_HOVER;
			if (!AnimationSelection.State) AnimationSelectionColor = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
		});

		// Set up AnimationSelection's on_click signal
		AnimationSelection.on_click.connect([&]()
		{
			// Need a drop down box here with all the options to be selected
			if (AnimationSelection.State == ButtonState::INACTIVE)
			{
				AnimationSelectionColor = EG::RGBA16(0.08f, 0.08f, 0.08f, 1.0f);
				AnimationSelection.State = ButtonState::ACTIVE;
			} 
			else
			{
				AnimationSelectionColor = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
				AnimationSelection.State = ButtonState::INACTIVE;
			} 
		});

		// Set up SceneAnimation's on_hover signal
		SceneAnimation.on_hover.connect([&]()
		{
			SceneAnimation.HoverState = HoveredState::ON_HOVER;
		});

		// Set up SceneAnimation's off_hover signal
		SceneAnimation.off_hover.connect([&]()
		{
			SceneAnimation.HoverState = HoveredState::OFF_HOVER;
		});

		// Set up InputText's on_click signal
		InputText.on_click.connect([&]()
		{
			// Naive way first - get mouse position in UI space
			EM::Vec2 MouseCoords = Input->GetMouseCoords();
			CameraManager::GetCamera("HUDCamera")->ConvertScreenToWorld(MouseCoords);

			std::string& Text = InputText.Text;
			auto XAdvance = InputText.Position.x;
			uint32_t index = 0;

			std::cout << "Mouse x: " << MouseCoords.x << std::endl;

			// Get advance
			for (auto& c : Text)
			{
				float Advance = EG::Fonts::GetAdvance(c, EG::FontManager::GetFont("WeblySleek"), 1.0f);
				if (XAdvance + Advance < MouseCoords.x) 
				{
					XAdvance += Advance;
					index++;
					std::cout << "XAdvance: " << XAdvance << std::endl;
					std::cout << "Index: " << index << std::endl;
				}
				else break;
			}

			InputText.CursorIndex = index;
			std::cout << "Cursor Index: " << InputText.CursorIndex << std::endl;

			// set caret on to true and count to 0
			caret_count = 0.0f;
			caret_on = true;
		});

		InputText.on_backspace.connect([&]()
		{
			auto str_len = InputText.Text.length();
			auto cursor_index = InputText.CursorIndex;

			// erase from string
			if (str_len > 0 && cursor_index > 0)
			{
				auto S1 = InputText.Text.substr(0, cursor_index - 1);
				std::string S2;

				if (cursor_index + 1 < str_len) S2 = InputText.Text.substr(cursor_index, str_len);

				S1.erase(cursor_index - 1);
				InputText.Text = S1 + S2;
				InputText.CursorIndex--;
			}
		});

		InputText.on_keyboard.connect([&](std::string c)
		{
			auto str_len = InputText.Text.length();
			auto cursor_index = InputText.CursorIndex;

			// std::cout << cursor_index << std::endl;

			// End of string
			if (cursor_index >= str_len)
			{
				InputText.Text += c;
				InputText.CursorIndex = str_len + 1;
			}
			// Cursor somewhere in the middle of the string
			else if (cursor_index > 0)
			{
				auto FirstHalf = InputText.Text.substr(0, cursor_index);
				auto SecondHalf = InputText.Text.substr(cursor_index, str_len);

				FirstHalf += c; 
				InputText.Text = FirstHalf + SecondHalf;
				InputText.CursorIndex++;
			}
			// Beginning of string
			else
			{
				InputText.Text = c + InputText.Text;
				InputText.CursorIndex++;
			}
		});

		// Set up InputText's on_hover signal
		InputText.on_hover.connect([&]()
		{
			// Change the mouse cursor
			SDL_SetCursor(CursorManager::Get("IBeam"));

			InputText.HoverState = HoveredState::ON_HOVER;

			// Change color of Box
			InputTextColor = EG::SetOpacity(EG::RGBA16_LightGrey(), 0.3f);

		});

		// Set up InputText's off_hover signal
		InputText.off_hover.connect([&]()
		{
			// Change mouse cursor back to defaul
			SDL_SetCursor(CursorManager::Get("Arrow"));

			InputText.HoverState = HoveredState::OFF_HOVER;
		
			// Change color of Box
			InputTextColor = EG::RGBA16(0.05f, 0.05f, 0.05f, 0.4f);
		});

		// Set up PlayButton's on_hover signal
		PlayButton.on_hover.connect([&]()
		{
			// We'll just change a color for now
			PlayButtonColor = EG::RGBA16_White();

			// Set state to active
			PlayButton.HoverState = HoveredState::ON_HOVER;
		});

		// Set up PlayButton's off_hover signal
		PlayButton.off_hover.connect([&]()
		{
			PlayButtonColor = EG::RGBA16_LightGrey();

			// Set state to inactive
			PlayButton.HoverState = HoveredState::OFF_HOVER;
		});

		// Set up PlayButton's signal
		PlayButton.on_click.connect([&]()
		{
			if (TimeIncrement <= 0.0f) 
			{
				TimeIncrement = 0.15f;
				PlayButton.State = ButtonState::ACTIVE;
			}

			else 
			{
				TimeIncrement = 0.0f;
				PlayButton.State = ButtonState::INACTIVE;
			}
		});

		// Set up NextFrame's signal
		NextFrame.on_click.connect([&]()
		{
			// If playing, then stop the time
			if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

			PlayButton.State = ButtonState::INACTIVE;

			// Get Current Animation, which in this case is just Test
			auto CurrentIndex = SceneAnimation.CurrentIndex;
			SceneAnimation.CurrentIndex = (CurrentIndex + 1) % SceneAnimation.CurrentAnimation->TotalFrames;

			// And set t = -1.0f for safety
			t = -1.0f;
		});

		// Set up PreviousFrame's signal
		PreviousFrame.on_click.connect([&]()
		{
			// If playing, then stop the time
			if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

			PlayButton.State = ButtonState::INACTIVE;

			// Get Current Animation, which in this case is just Test
			auto CurrentIndex = SceneAnimation.CurrentIndex;
			if (CurrentIndex > 0) SceneAnimation.CurrentIndex -= 1;

			// Bounds check
			else SceneAnimation.CurrentIndex = SceneAnimation.CurrentAnimation->TotalFrames - 1;

			// And set t = -1.0f for safety
			t = -1.0f;
		});

		// Set up OffsetUp's signal
		OffsetUp.on_click.connect([&]()
		{
			// If playing, then stop the time
			if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

			PlayButton.State = ButtonState::INACTIVE;

			// Get Current Frame
			auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

			// Get CurrentFrame's YOffset
			auto YOffset = CurrentFrame->Offsets.y;

			// Increment by arbitrary amount...
			YOffset += 1.0f;

			// Reset offset
			CurrentFrame->Offsets.y = YOffset;
		});

		// Set up OffsetDown's signal
		OffsetDown.on_click.connect([&]()
		{
			// If playing, then stop the time
			if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

			PlayButton.State = ButtonState::INACTIVE;

			// Get Current Frame
			auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

			// Get CurrentFrame's YOffset
			auto YOffset = CurrentFrame->Offsets.y;

			// Increment by arbitrary amount...
			YOffset -= 1.0f;

			// Reset offset
			CurrentFrame->Offsets.y = YOffset;
		});

		// Set up OffsetLeft's signal
		OffsetLeft.on_click.connect([&]()
		{
			// If playing, then stop the time
			if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

			PlayButton.State = ButtonState::INACTIVE;

			// Get Current Frame
			auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

			// Get CurrentFrame's YOffset
			auto XOffset = CurrentFrame->Offsets.x;

			// Increment by arbitrary amount...
			XOffset -= 1.0f;

			// Reset offset
			CurrentFrame->Offsets.x = XOffset;
		});

		// Set up OffsetRight's signal
		OffsetRight.on_click.connect([&]()
		{
			// If playing, then stop the time
			if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

			PlayButton.State = ButtonState::INACTIVE;

			// Get Current Frame
			auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

			// Get CurrentFrame's YOffset
			auto XOffset = CurrentFrame->Offsets.x;

			// Increment by arbitrary amount...
			XOffset += 1.0f;

			// Reset offset
			CurrentFrame->Offsets.x = XOffset;
		});

		// Set up DelayUp's signal
		DelayUp.on_click.connect([&]()
		{
			// If playing, then stop the time
			if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

			PlayButton.State = ButtonState::INACTIVE;

			// Get Current Frame
			auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

			// Get CurrentFrame's Delay
			auto Delay = CurrentFrame->Delay;

			// Increment by arbitrary amount...
			Delay += 0.1f;

			// Reset Delay
			CurrentFrame->Delay = Delay;
		});

		// Set up DelayUp's signal
		DelayDown.on_click.connect([&]()
		{
			// If playing, then stop the time
			if (TimeIncrement != 0.0f) TimeIncrement = 0.0f;

			PlayButton.State = ButtonState::INACTIVE;

			// Get Current Frame
			auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

			// Get CurrentFrame's Delay
			auto Delay = CurrentFrame->Delay;

			// Increment by arbitrary amount...
			if (Delay > 0.0f) Delay -= 0.1f;

			// Reset Delay
			CurrentFrame->Delay = Delay;
		});

		// Put into button manager map
		ButtonManager::Add("PlayButton", &PlayButton);
		ButtonManager::Add("NextFrame", &NextFrame);
		ButtonManager::Add("PreviousFrame", &PreviousFrame);
		ButtonManager::Add("OffsetUp", &OffsetUp);
		ButtonManager::Add("OffsetDown", &OffsetDown);
		ButtonManager::Add("OffsetLeft", &OffsetLeft);
		ButtonManager::Add("OffsetRight", &OffsetRight);
		ButtonManager::Add("DelayUp", &DelayUp);
		ButtonManager::Add("DelayDown", &DelayDown);

		// Put into textbox manager
		TextBoxManager::Add("InputText", &InputText);

		GUIManager::Add("PlayButton", &PlayButton);
		GUIManager::Add("NextFrame", &NextFrame);
		GUIManager::Add("PreviousFrame", &PreviousFrame);
		GUIManager::Add("OffsetUp", &OffsetUp);
		GUIManager::Add("OffsetDown", &OffsetDown);
		GUIManager::Add("OffsetLeft", &OffsetLeft);
		GUIManager::Add("OffsetRight", &OffsetRight);
		GUIManager::Add("DelayUp", &DelayUp);
		GUIManager::Add("DelayDown", &DelayDown);
		GUIManager::Add("InputText", &InputText);
		GUIManager::Add("SceneAnimation", &SceneAnimation);
		GUIManager::Add("ToggleOnionSkin", &ToggleOnionSkin);
		GUIManager::Add("AnimationSelection", &AnimationSelection);

		// Draw BG
		BGBatch->Begin();
		// BGBatch->Add(
		// 				EM::Vec4(-SCREENWIDTH / 2.0f, -SCREENHEIGHT / 2.0f, SCREENWIDTH, SCREENHEIGHT),
		// 				EM::Vec4(0, 0, 1, 1),
		// 				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/bg.png", GL_LINEAR).id,
		// 				EG::SetOpacity(EG::RGBA16_SkyBlue(), 0.4f)
		// 			);
		// BGBatch->Add(
		// 				EM::Vec4(-SCREENWIDTH / 2.0f, -SCREENHEIGHT / 2.0f, SCREENWIDTH, SCREENHEIGHT),
		// 				EM::Vec4(0, 0, 1, 1),
		// 				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/bg.png", GL_LINEAR).id,
		// 				EG::SetOpacity(EG::RGBA16_White(), 0.3f)
		// 			);
		BGBatch->Add(
						EM::Vec4(-SCREENWIDTH / 2.0f, -SCREENHEIGHT / 2.0f, SCREENWIDTH, SCREENHEIGHT),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/bg_cross.png", GL_LINEAR).id,
						EG::SetOpacity(EG::RGBA16_White(), 0.1f)
					);
		BGBatch->End();

		
		return true;	
	}

	bool Update()
	{
		// Keep track of animation delays
		t += TimeIncrement * TimeScale;

		// Check for quit condition
		IsRunning = ProcessInput(Input, &Camera);

		// Update cameras
		Camera.Update();
		HUDCamera.Update();

		// Set up AABB of Scene Animation
		EGUI::AnimationElement::AABBSetup(&SceneAnimation);

		// Set up AABB's of AnimationSelection list elements
		auto Offset = 20.0f;
		auto Position = AnimationSelection.AABB.Min;
		auto Dimensions = AnimationSelection.Dimensions;
		for (auto e : AnimationSelection.List)
		{
			auto AABB_e = &e->AABB;

			// UIBatch->Add(
			// 				EM::Vec4(
			// 							HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + XOffset - XPadding,
			// 							HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, 
			// 							GroupWidth, 20.0f
			// 						),
			// 				EM::Vec4(0, 0, 1, 1),
			// 				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id, 
			// 				AnimationSelection.List.at(i).Color
			// 			);

			// Calculate min
			AABB_e->Min = EM::Vec2(Position - EM::Vec2(0.0f, Offset));

			// Calculate max
			AABB_e->Max = AABB_e->Min + EM::Vec2(200.0f, 20.0f);


			// Offset += 20.0f;

			// Add to position
			Position.y -= Offset;
		}

		// Update input
		Input->Update();

		return IsRunning;
	}		

	bool Draw()
	{
		// Set up necessary matricies
    	auto Model 		= EM::Mat4::Identity();	
    	auto View 		= HUDCamera.GetCameraMatrix();
    	auto Projection = EM::Mat4::Identity();

    	// Set up clipping mask for entire Animation Info group


		// Basic shader for UI
		BasicShader->Use();
		{
			BasicShader->SetUniformMat4("model", Model);
			BasicShader->SetUniformMat4("projection", Projection);
			BasicShader->SetUniformMat4("view", View);

			// Draw BG
			BGBatch->RenderBatch();

			UIBatch->Begin();
			{
				// Draw Parent
				auto Parent = PlayButton.Parent;

				// UIBatch->Add(
				// 				EM::Vec4(Parent->AABB.Min, Parent->AABB.Max - Parent->AABB.Min),
				// 				EM::Vec4(0, 0, 1, 1),
				// 				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
				// 				EG::SetOpacity(EG::RGBA16_Blue(), 0.05f)
				// 			);

				// Draw Play button
				auto PBF = PlayButton.Frames.at(PlayButton.State);
				// Calculate these offsets
				CalculateAABBWithParent(&PlayButton.AABB, &PlayButton);
				DrawFrame(PBF, PlayButton.Position + Parent->Position, UIBatch, PlayButtonColor);

				// Draw input text
				UIBatch->Add(
								EM::Vec4(InputText.AABB.Min, InputText.AABB.Max - InputText.AABB.Min), 
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								InputTextColor
							);


			}
			UIBatch->End();
			UIBatch->RenderBatch();

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
				if (ToggleOnionSkin.State == ButtonState::ACTIVE)
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

		// Shader for text
		TextShader->Use();
		{
			View = HUDCamera.GetCameraMatrix();

			TextShader->SetUniformMat4("model", Model);
			TextShader->SetUniformMat4("projection", Projection);
			TextShader->SetUniformMat4("view", View);

			// glEnable(GL_SCISSOR_TEST);
			UIBatch->Begin(EG::GlyphSortType::FRONT_TO_BACK);
			{
				// Get font for use
				auto CurrentFont = EG::FontManager::GetFont("WeblySleek_12");
				auto XOffset = 110.0f;
				auto scale = 1.0f;
				auto YOffset = 70.0f;
				auto ClipYOffset = YOffset - 25.0f;
				auto Difference = YOffset - ClipYOffset;

				auto CurrentFrame = &SceneAnimation.CurrentAnimation->Frames.at(SceneAnimation.CurrentIndex);

				// Scissor out entire info area
				auto ClipWidth = 300.0f;
				auto ClipHeight = 350.0f;
				// glScissor(
				// 			HUDCamera.GetPosition().x + 2.0f, 
				// 			HUDCamera.GetPosition().y + SCREENHEIGHT - ClipHeight, 
				// 			ClipWidth, 
				// 			ClipHeight
				// 		);

				EM::Vec2 TitleBarBL(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 5.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - ClipYOffset - Difference);

				// Draw shadow
				UIBatch->Add(
								EM::Vec4(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 5.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - ClipHeight - ClipYOffset - 5.0f, ClipWidth, ClipHeight - Difference),
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								EG::SetOpacity(EG::RGBA16_Black(), 0.5f)
							);

				// Draw Widget border
				EG::DrawRectBorder	(
										UIBatch,
										EM::Vec4(
													HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 5.0f, 
													HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - ClipHeight - ClipYOffset, 
													ClipWidth, 
													ClipHeight
												),
										1.0f,
										EG::SetOpacity(EG::RGBA16_DarkGrey(), 1.0f)
									); 


				// Draw Title Bar
				UIBatch->Add(
								EM::Vec4(TitleBarBL.x, TitleBarBL.y, ClipWidth, Difference),
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f),
								-100.0f
							);

				// Draw title of widget
				std::string WidgetTitle("Animation Editor");

				auto TitleFont = EG::FontManager::GetFont("WeblySleek");
				// Calculate total width of title to find placement
				float TitleAdvance = 0.0f;
				for (auto& c : WidgetTitle)
				{
					TitleAdvance += EG::Fonts::GetAdvance(c, TitleFont, 1.0f);
				}

				EG::Fonts::PrintText(
										TitleBarBL.x + ClipWidth / 2.0f - TitleAdvance / 2.0f,
										TitleBarBL.y + Difference / 3.0f,
										1.0f,
										WidgetTitle,
										TitleFont,
										*UIBatch,
										EG::RGBA16_MidGrey()
									);


				// Draw the clipping area
				UIBatch->Add(
								EM::Vec4(HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 5.0f, HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - ClipHeight - ClipYOffset, ClipWidth, ClipHeight - Difference),
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								EG::RGBA16(0.12, 0.12, 0.12, 1.0f)
							);


				// Draw Title border
				EG::DrawRectBorder	(
										UIBatch,
										EM::Vec4(
													TitleBarBL.x, 
													TitleBarBL.y, 
													ClipWidth, 
													Difference
												),
										1.5f,
										EG::SetOpacity(EG::RGBA16(0.1f, 0.1f, 0.1f, 1.0f), 0.6f)
									); 

				YOffset += 25.0f;

				// Display current frame information
				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										std::string("Animation: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_MidGrey()
									);

				// Draw animation selection AABB
				auto Padding = EM::Vec2(5.0f, 5.0f);

				// Draw Border of animation selection AABB
				EG::DrawRectBorder	(
										UIBatch,
										EM::Vec4(
													AnimationSelection.AABB.Min, 
													AnimationSelection.AABB.Max + EM::Vec2(4.0f * Padding.x, 0.0f) - AnimationSelection.AABB.Min 
												),
										1.0f,
										EG::SetOpacity(EG::RGBA16(0.18, 0.18, 0.18, 1.0f), 0.5f)
									); 

				UIBatch->Add(
								EM::Vec4(AnimationSelection.AABB.Min, AnimationSelection.AABB.Max + EM::Vec2(4.0f * Padding.x, 0.0f) - AnimationSelection.AABB.Min),
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								AnimationSelectionColor	
							);

				// Print out AnimationSelection text
				{
					auto ATextHeight = AnimationSelection.AABB.Max.y - AnimationSelection.AABB.Min.y;
					auto TextHeight = ATextHeight - 20.0f;
					EG::Fonts::PrintText(	
											AnimationSelection.AABB.Min.x + Padding.x, 
											AnimationSelection.AABB.Min.y + Padding.y, 1.0f, 
											AnimationSelection.Text, 
											CurrentFont, 
											*UIBatch, 
											EG::RGBA16_White()
										);

				}

				auto AnimationSelectionYOffset = YOffset;	
				
				YOffset += 20.0f;	
				// Current Frame Name
				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										std::string("Frame: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_MidGrey()
									);



				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										std::to_string(SceneAnimation.CurrentIndex), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_White()
									);
				YOffset += 20.0f;	
				// Current Frame Delay
				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										std::string("Delay: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_MidGrey()
									);
				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										std::to_string(CurrentFrame->Delay), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_White()
									);
				YOffset += 20.0f;	
				// Current Frame Y offset
				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										std::string("Y Offset: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_MidGrey()
									);
				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										std::to_string(static_cast<int32_t>(CurrentFrame->Offsets.y)), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_White()
									);
				YOffset += 20.0f;	
				// Current Frame X Offset
				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										std::string("X Offset: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_MidGrey()
									);
				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										std::to_string(static_cast<int32_t>(CurrentFrame->Offsets.x)), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_White()
									);
				YOffset += 20.0f;	
				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										std::string("Onion Skin: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_MidGrey()
									);
				auto OnionString = ToggleOnionSkin.State == ButtonState::ACTIVE ? std::string("On") : std::string("Off");
				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										OnionString, 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_White()
									);

				YOffset += 20.0f;	
				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + 15.0f, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										std::string("Time Scale: "), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_MidGrey()
									);
				EG::Fonts::PrintText(	
										HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + XOffset, 
										HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, scale, 
										std::to_string(TimeScale), 
										CurrentFont, 
										*UIBatch, 
										EG::RGBA16_White()
									);

				if (AnimationSelection.State)
				{
					YOffset = AnimationSelectionYOffset;
					YOffset += 5.0f;
					float XPadding = 5.0f;
					auto Amount = AnimationSelection.List.size();
					auto GroupWidth = 185.0f;

					// Draw shadow for group for now
					UIBatch->Add(
									EM::Vec4(
												HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + XOffset - XPadding + 5.0f,
												HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset - 20.0f * Amount - 5.0f, 
												GroupWidth, 20.0f * Amount
											),
									EM::Vec4(0, 0, 1, 1),
									EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id, 
									EG::SetOpacity(EG::RGBA16_Black(), 0.2f)
								);

					// Draw border for group for now
					EG::DrawRectBorder	(
											UIBatch, 
											EM::Vec4(
														HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + XOffset - XPadding,
														HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset - 20.0f * Amount, 
														GroupWidth, 
														20.0f * Amount
													),
											1.0f,
											EG::RGBA16_DarkGrey()
										);

					auto F = EG::FontManager::GetFont("WeblySleek_10");
					for (auto i = 0; i < Amount; ++i)
					{
						YOffset += 20.0f;

						// Draw box
						// UIBatch->Add(
						// 				EM::Vec4(
						// 							HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + XOffset - XPadding,
						// 							HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, 
						// 							GroupWidth, 20.0f
						// 						),
						// 				EM::Vec4(0, 0, 1, 1),
						// 				EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id, 
						// 				AnimationSelection.List.at(i).Color
						// 			);

						// Draw AABB of list element
						UIBatch->Add(
										EM::Vec4(
													AnimationSelection.List.at(i)->AABB.Min,
													AnimationSelection.List.at(i)->AABB.Max - AnimationSelection.List.at(i)->AABB.Min
												),
										EM::Vec4(0, 0, 1, 1),
										EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id, 
										AnimationSelection.List.at(i)->Color	
									);

						EG::Fonts::PrintText(	
												HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + XOffset + XPadding, 
												HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset + 6.0f, scale, 
												AnimationSelection.List.at(i)->Text, 
												F, 
												*UIBatch, 
												EG::RGBA16_MidGrey()
											);

						EG::DrawRectBorder	(
												UIBatch, 
												EM::Vec4(
															HUDCamera.GetPosition().x - SCREENWIDTH / 2.0f + XOffset - XPadding,
															HUDCamera.GetPosition().y + SCREENHEIGHT / 2.0f - YOffset, 
															GroupWidth, 
															20.0f
														),
												1.0f,
												EG::RGBA16(0.06f, 0.06f, 0.06f, 1.0f)
											);
					}
				}



			}
			UIBatch->End();
			UIBatch->RenderBatch();
			glDisable(GL_SCISSOR_TEST);

			caret_count += 0.1f;

			UIBatch->Begin();
			{
				// Print out text box's text w/ shadow
				// Could totally load these styles from JSON, which would be a cool way to add themes to the editor
				auto Padding = EM::Vec2(5.0f, 5.0f);

				{
					auto ITextHeight = InputText.AABB.Max.y - InputText.AABB.Min.y; // InputTextHeight
					auto TextHeight = ITextHeight - 20.0f;
					EG::Fonts::PrintText(	
											InputText.Position.x + InputText.Parent->Position.x + Padding.x, 
											InputText.Position.y + InputText.Parent->Position.y + Padding.y + TextHeight, 1.0f, 
											InputText.Text, 
											EG::FontManager::GetFont("WeblySleek"), 
											*UIBatch, 
											EG::RGBA16_LightGrey()
										);
				}
				if (caret_count >= 4.0f)
				{
					caret_count = 0.0f;
					caret_on = !caret_on;	
				}

				if (KeyboardFocus && caret_on)
				{
					// Print out caret, make it a yellow line
					// Need to get text from InputText
					auto CurrentFont = EG::FontManager::GetFont("WeblySleek");
					auto scale = 1.0f;
					auto Padding = EM::Vec2(5.0f, 5.0f);
					auto Text = InputText.Text;
					auto XAdvance = InputText.Position.x + InputText.Parent->Position.x + Padding.x;
					auto ITextHeight = InputText.AABB.Max.y - InputText.AABB.Min.y; // InputTextHeight
					auto TextHeight = ITextHeight - 20.0f;

					// Get xadvance of all characters
					for (auto i = 0; i < InputText.CursorIndex; ++i)
					{
						XAdvance += EG::Fonts::GetAdvance(InputText.Text[i], CurrentFont, scale);
					}
					UIBatch->Add(
									EM::Vec4(XAdvance + 0.2f, InputText.Position.y + InputText.Parent->Position.y + Padding.y + TextHeight, 1.0f, 10.0f),
									EM::Vec4(0, 0, 1, 1),
									EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
									EG::RGBA16_LightGrey()
								);
				}
			}
			UIBatch->End();
			UIBatch->RenderBatch();
		}

		TextShader->Unuse();

		BasicShader->Use();
		BasicShader->SetUniformMat4("view", HUDCamera.GetCameraMatrix());
		{
			UIBatch->Begin();
			{
				UIBatch->Add(
								EM::Vec4(
											AnimationSelection.AABB.Max - EM::Vec2(3.0f, 18.0f), 
											EM::Vec2(16, 16)
									     ),
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../Assets/Textures/gui_down_arrow.png").id,
								EG::RGBA16_White()	
							);
			}
			UIBatch->End();
			UIBatch->RenderBatch();
		}
		BasicShader->Unuse();
	
		return true;
	}


	bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera)
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

	    if (SelectedGUIElement)
	    {
	    	switch (SelectedGUIElement->Type)
	    	{
	    		case GUIType::BUTTON:
	    			// std::cout << "Selected Button!" << std::endl;
	    			break;
	    		case GUIType::TEXTBOX:
	    			// std::cout << "Selected TextBox!" << std::endl;
	    			break;
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
						if (ListElement->HoverState == HoveredState::ON_HOVER) ListElement->off_hover.emit(ListElement);
					}

					// Assign list element
					ListElement = e;

					// If not hoverstate active, then emit
					if (ListElement->HoverState == HoveredState::OFF_HOVER) 
					{
						ListElement = e;
						e->on_hover.emit(e);
					}

					// Found, so break out of loop
					Found = true;
					break;
				}
			}
			// Nothing was selected but previous list element assigned
			if (!Found && ListElement)
			{
				ListElement->off_hover.emit(ListElement);
				ListElement = nullptr;
			}
		}
		// If there was something, need to get rid of it
		else if (ListElement)
		{
			ListElement->off_hover.emit(ListElement);
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

	    	// Do AABB test with AnimationSelection List Element
	    	else if (ListElement)
	    	{
	    		ListElement->on_click.emit(ListElement);
	    		ListElement->off_hover.emit(ListElement);
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
	    		InputText->on_click.emit();
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
		    		std::cout << "Here..." << std::endl;
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
				auto PlayButton = ButtonManager::Get("PlayButton");

				// Press play
				PlayButton->on_click.emit();
			}
			if (Input->IsKeyDown(SDLK_RIGHT))
			{
				// Get button from button manager
				auto OffsetRight = ButtonManager::Get("OffsetRight");

				// Press next frame
				OffsetRight->on_click.emit();	
			}
			if (Input->IsKeyDown(SDLK_LEFT))
			{
				// Get button from button manager
				auto OffsetLeft = ButtonManager::Get("OffsetLeft");

				// Press next frame
				OffsetLeft->on_click.emit();
			}
			if (Input->IsKeyDown(SDLK_UP))
			{
				// Get button from button manager
				auto OffsetUp = ButtonManager::Get("OffsetUp");

				// Press offset up
				OffsetUp->on_click.emit();
			}
			if (Input->IsKeyDown(SDLK_DOWN))
			{
				// Get button from button manager
				auto OffsetDown = ButtonManager::Get("OffsetDown");

				// Press offset Down
				if (OffsetDown)
				{
					OffsetDown->on_click.emit();
				}
			}
			if (Input->IsKeyPressed(SDLK_m))
			{
				// Get button from button manager
				auto NextFrame = ButtonManager::Get("NextFrame");

				// Press offset Down
				if (NextFrame)
				{
					NextFrame->on_click.emit();
				}
			}
			if (Input->IsKeyPressed(SDLK_n))
			{
				// Get button from button manager
				auto PreviousFrame = ButtonManager::Get("PreviousFrame");

				// Press offset Down
				if (PreviousFrame)
				{
					PreviousFrame->on_click.emit();
				}

			}
			if (Input->IsKeyPressed(SDLK_LEFTBRACKET))
			{
				// Get button from button manager
				auto DelayDown = ButtonManager::Get("DelayDown");

				// Emit
				if (DelayDown)
				{
					DelayDown->on_click.emit();	
				}	
			}
			if (Input->IsKeyPressed(SDLK_RIGHTBRACKET))
			{
				// Get button from button manager
				auto DelayUp = ButtonManager::Get("DelayUp");

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

}}
