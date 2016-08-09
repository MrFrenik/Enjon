#ifndef GUIELEMENT_H
#define GUIELEMENT_H

#include "Defines.h" 

#include "Graphics/Color.h"
#include "Graphics/Font.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/Camera2D.h"
#include "Graphics/FontManager.h"
#include "Graphics/CursorManager.h"
#include "IO/ResourceManager.h"
#include "IO/InputManager.h"
#include "Math/Vec2.h"
#include "Math/Vec4.h"
#include "Physics/AABB.h"
#include "GUI/Signal.h"
#include "GUI/Property.h"

#include <SDL2/SDL.h>


#include <vector>
#include <string>
#include <cstdint>

namespace Enjon { namespace GUI {

	enum ButtonState { INACTIVE, ACTIVE };
	enum HoveredState { OFF_HOVER, ON_HOVER };
	enum GUIType { BUTTON, TEXT_BOX, SCENE_ANIMATION, GROUP, TEXT_BUTTON , DROP_DOWN_BUTTON, RADIAL_BUTTON, VALUE_BUTTON };
	enum VisibleState { HIDDEN, VISIBLE };
	enum ClickState { NOT_CLICKABLE, CLICKABLE };

	// GUI Element
	struct GUIElementBase
	{
		virtual void Init() = 0;
		virtual void Update() = 0;
		virtual void Draw(EG::SpriteBatch* TextBatch) = 0;
		virtual bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera) = 0;

		GUIElementBase* Parent;
		EM::Vec2 Position;
		EP::AABB AABB;
		GUIType Type;

		EM::Vec2 Dimensions;
		std::string Text;
		std::string Name;
		ButtonState State;
		HoveredState HoverState;
		VisibleState Visibility;
		ClickState Clickability;
		EGUI::Signal<> on_click;
		EGUI::Signal<> on_hover;
		EGUI::Signal<> off_hover;
		EGUI::Signal<> lose_focus;
		EGUI::Signal<EI::InputManager*, EG::Camera2D*> check_children;
		EG::ColorRGBA16 Color;
		EG::ColorRGBA16 BorderColor;
		EG::ColorRGBA16 TextColor;
		uint32_t JustFocused;
		float Depth;
	};

	template <typename T>
	struct GUIElement : public GUIElementBase
	{
		void Init()
		{
			static_cast<T*>(this)->Init();

			// By default
			static_cast<T*>(this)->Clickability = ClickState::CLICKABLE;
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			static_cast<T*>(this)->Draw(Batch);
		}

		void Update()
		{
			static_cast<T*>(this)->Update();
		}

		bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera)
		{
			return static_cast<T*>(this)->ProcessInput(Input, Camera);
		}
	};


	// TextBox
	struct GUITextBox : GUIElement<GUITextBox>
	{
		GUITextBox() 
		{ 
			// Set up type
			Type = GUIType::TEXT_BOX; 

			// Initialize members
			Name 			= std::string("GUITextBox");
			Text 			= std::string("");
			CursorIndex 	= 0;
			TextColor 		= EG::RGBA16_White();
			Color 	 		= EG::RGBA16_DarkGrey();
			BorderColor 	= Color;
			caret_on 		= false;
			caret_count 	= 0.0f;
			TextFont 		= nullptr;
			KeyboardInFocus = false;
			Dimensions 		= EM::Vec2(100.0f, 20.0f);
			MaxStringLength = 20;

			// Initial states
			State 		= ButtonState::INACTIVE;
			HoverState 	= HoveredState::OFF_HOVER;

			// Get font
			FontScale = 1.0f;

			// Set up TextBox's on_hover signal
			this->on_hover.connect([&]()
			{
				// Change the mouse cursor
				SDL_SetCursor(EG::CursorManager::Get("IBeam"));

				HoverState = HoveredState::ON_HOVER;

				// Change color of Box
				Color = EG::SetOpacity(EG::RGBA16(0.2f, 0.2f, 0.2f, 1.0f), 0.3f);
			});

			// Set up TextBox's off_hover signal
			this->off_hover.connect([&]()
			{
				// Change mouse cursor back to defaul
				SDL_SetCursor(EG::CursorManager::Get("Arrow"));

				HoverState = HoveredState::OFF_HOVER;
			
				// Change color of Box
				Color = EG::RGBA16_DarkGrey();
			});

			// Set up TextBox's on_keyboard signal
			this->on_keyboard.connect([&](std::string c)
			{
				auto str_len = Text.length();
				auto cursor_index = CursorIndex;

				// Don't exceed max length
				if (str_len >= MaxStringLength) return;

				// End of string
				if (cursor_index >= str_len)
				{
					Text += c;
					CursorIndex = str_len + 1;
				}
				// Cursor somewhere in the middle of the string
				else if (cursor_index > 0)
				{
					auto FirstHalf = Text.substr(0, cursor_index);
					auto SecondHalf = Text.substr(cursor_index, str_len);

					FirstHalf += c; 
					Text = FirstHalf + SecondHalf;
					CursorIndex++;
				}
				// Beginning of string
				else
				{
					Text = c + Text;
					CursorIndex++;
				}
			});

			// Set up TextBox's on_backspace signal
			this->on_backspace.connect([&]()
			{
				auto str_len = Text.length();
				auto cursor_index = CursorIndex;

				// erase from string
				if (str_len > 0 && cursor_index > 0)
				{
					auto S1 = Text.substr(0, cursor_index - 1);
					std::string S2;

					if (cursor_index + 1 < str_len) S2 = Text.substr(cursor_index, str_len);

					S1.erase(cursor_index - 1);
					Text = S1 + S2;
					CursorIndex--;
				}
			});

			this->on_click.connect([&](float MouseX)
			{
				auto XAdvance = Position.x;
				uint32_t index = 0;

				// Set border color to active
				BorderColor 	= EG::SetOpacity(EG::RGBA16(0.20f, 0.635f, 1.0f, 1.0f), 0.5f);

				// Get advance
				for (auto& c : Text)
				{
					float Advance = EG::Fonts::GetAdvance(c, TextFont, FontScale);
					if (XAdvance + Advance < MouseX) 
					{
						XAdvance += Advance;
						index++;
					}
					else break;
				}
				CursorIndex = index;

				// set caret on to true and count to 0
				caret_count = 0.0f;
				caret_on = true;

				KeyboardInFocus = true;
			});

			this->lose_focus.connect([&]()
			{
				KeyboardInFocus = false;
				caret_on 		= false;
				BorderColor 	= EG::SetOpacity(Color, 0.5f);
			});

			this->on_enter.connect([&]()
			{
				// Lose focus
				this->lose_focus.emit();
			});

		}

		void Update()
		{
			caret_count += 0.1f;

			if (KeyboardInFocus)
			{
				if (caret_count >= 4.0f)
				{
					caret_count = 0.0f;
					caret_on = !caret_on;	
				}
			}
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			auto Padding = EM::Vec2(5.0f, 5.0f);

			// Make sure font isn't null
			if (TextFont == nullptr) TextFont = EG::FontManager::GetFont("WeblySleek_12");

			// Draw box		
			Batch->Add(
						EM::Vec4(AABB.Min, AABB.Max - AABB.Min),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						Color, 
						0.0f
						// EG::SpriteBatch::DrawOptions::BORDER, 
						// BorderColor
					);

			// Print text
			auto ITextHeight = AABB.Max.y - AABB.Min.y; // InputTextHeight
			auto TextHeight = ITextHeight - 20.0f;
			EG::Fonts::PrintText(	
									AABB.Min.x + Padding.x, 
									AABB.Min.y + Padding.y, 1.0f, 
									Text, 
									TextFont, 
									*Batch, 
									EG::RGBA16_LightGrey()
								);
			
			// Draw Caret if on
			if (KeyboardInFocus && caret_on)
			{
				auto CurrentFont = TextFont;
				auto scale = FontScale;
				auto Padding = EM::Vec2(5.0f, 7.0f);
				auto XAdvance = Position.x + Padding.x;
				auto ITextHeight = AABB.Max.y - AABB.Min.y; // InputTextHeight
				auto TextHeight = ITextHeight - 20.0f;

				// Get xadvance of all characters
				for (auto i = 0; i < CursorIndex; ++i)
				{
					XAdvance += EG::Fonts::GetAdvance(Text[i], CurrentFont, scale);
				}
				Batch->Add(
								EM::Vec4(XAdvance + 0.2f, Position.y + Padding.y + TextHeight, 1.0f, 10.0f),
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								EG::RGBA16_LightGrey()
							);
			}

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

		std::string GetNumericString(EI::InputManager* Input)
		{
			std::string str = "";

			// Quick and dirty way so far...
			if (Input->IsKeyPressed(SDLK_0))
			{
				str += "0";
			}
			if (Input->IsKeyPressed(SDLK_1))
			{
				str += "1";
			}
			if (Input->IsKeyPressed(SDLK_2))
			{
				str += "2";
			}
			if (Input->IsKeyPressed(SDLK_3))
			{
				str += "3";
			}
			if (Input->IsKeyPressed(SDLK_4))
			{
				str += "4";
			}
			if (Input->IsKeyPressed(SDLK_5))
			{
				str += "5";
			}
			if (Input->IsKeyPressed(SDLK_6))
			{
				str += "6";
			}
			if (Input->IsKeyPressed(SDLK_7))
			{
				str += "7";
			}
			if (Input->IsKeyPressed(SDLK_8))
			{
				str += "8";
			}
			if (Input->IsKeyPressed(SDLK_9))
			{
				str += "9";
			}

			return str;
		}

		void Init()
		{}

		bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera)
		{
			return true;
		}

		ButtonState State;
		HoveredState HoverState;
		std::string Text;
		EG::Fonts::Font* TextFont;
		float FontScale;
		float caret_count;
		int32_t CursorIndex;
		int32_t caret_on;
		int32_t KeyboardInFocus;
		uint32_t MaxStringLength;

	
		EGUI::Signal<float> on_click;	
		EGUI::Signal<std::string> on_keyboard;
		EGUI::Signal<> on_backspace;
		EGUI::Signal<> on_enter;
	};


}}


#endif