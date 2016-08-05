#ifndef GUIELEMENT_H
#define GUIELEMENT_H

#include "Defines.h" 

#include "Graphics/Color.h"
#include "Graphics/Font.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/FontManager.h"
#include "Graphics/CursorManager.h"
#include "IO/ResourceManager.h"
#include "Math/Vec2.h"
#include "Math/Vec4.h"
#include "Physics/AABB.h"

#include "GUI/Signal.h"
#include "GUI/Property.h"


#include <vector>
#include <string>
#include <cstdint>

namespace Enjon { namespace GUI {

	enum ButtonState { INACTIVE, ACTIVE };
	enum HoveredState { OFF_HOVER, ON_HOVER };
	enum GUIType { BUTTON, TEXT_BOX, SCENE_ANIMATION, GROUP, TEXT_BUTTON , DROP_DOWN_BUTTON, RADIAL_BUTTON, VALUE_BUTTON };

	// GUI Element
	struct GUIElementBase
	{
		virtual void Init() = 0;
		virtual void Update() = 0;
		virtual void Draw(EG::SpriteBatch* TextBatch) = 0;

		GUIElementBase* Parent;
		EM::Vec2 Position;
		EP::AABB AABB;
		GUIType Type;

		EM::Vec2 Dimensions;
		std::string Text;
		std::string Name;
		ButtonState State;
		HoveredState HoverState;
		EGUI::Signal<> on_click;
		EGUI::Signal<> on_hover;
		EGUI::Signal<> off_hover;
		EG::ColorRGBA16 Color;
		float Depth;
	};

	template <typename T>
	struct GUIElement : public GUIElementBase
	{
		void Init()
		{
			static_cast<T*>(this)->Init();
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			static_cast<T*>(this)->Draw(Batch);
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
			TextColor 	= EG::RGBA16_White();
			Color 	 	= EG::RGBA16_DarkGrey();
			caret_on 		= false;
			caret_count 	= 0.0f;
			TextFont 		= nullptr;
			KeyboardInFocus = false;
			Dimensions 		= EM::Vec2(100.0f, 20.0f);

			// Initial states
			State 		= ButtonState::INACTIVE;
			HoverState 	= HoveredState::OFF_HOVER;

			// Get font
			FontScale = 1.0f;

			// Set up TextBox's on_hover signal
			on_hover.connect([&]()
			{
				// Change the mouse cursor
				SDL_SetCursor(EG::CursorManager::Get("IBeam"));

				HoverState = HoveredState::ON_HOVER;

				// Change color of Box
				Color = EG::SetOpacity(EG::RGBA16_LightGrey(), 0.3f);

			});

			// Set up TextBox's off_hover signal
			off_hover.connect([&]()
			{
				// Change mouse cursor back to defaul
				SDL_SetCursor(EG::CursorManager::Get("Arrow"));

				HoverState = HoveredState::OFF_HOVER;
			
				// Change color of Box
				Color = EG::RGBA16_DarkGrey();
			});

			// Set up TextBox's on_keyboard signal
			on_keyboard.connect([&](std::string c)
			{
				auto str_len = Text.length();
				auto cursor_index = CursorIndex;

				// std::cout << cursor_index << std::endl;

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
			on_backspace.connect([&]()
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

			on_click.connect([&](float MouseX)
			{
				auto XAdvance = Position.x;
				uint32_t index = 0;

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
		}

		void Update()
		{
			caret_count += 0.1f;


			if (caret_count >= 4.0f)
			{
				caret_count = 0.0f;
				caret_on = !caret_on;	
			}
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			auto Padding = EM::Vec2(5.0f, 5.0f);

			if (TextFont == nullptr) TextFont = EG::FontManager::GetFont("WeblySleek_12");

			{
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
			}
			
			if (KeyboardInFocus && caret_on)
			{
	
				auto CurrentFont = TextFont;
				auto scale = FontScale;
				auto Padding = EM::Vec2(5.0f, 5.0f);
				auto XAdvance = Position.x + Parent->Position.x + Padding.x;
				auto ITextHeight = AABB.Max.y - AABB.Min.y; // InputTextHeight
				auto TextHeight = ITextHeight - 20.0f;

				// Get xadvance of all characters
				for (auto i = 0; i < CursorIndex; ++i)
				{
					XAdvance += EG::Fonts::GetAdvance(Text[i], CurrentFont, scale);
				}
				Batch->Add(
								EM::Vec4(XAdvance + 0.2f, Position.y + Parent->Position.y + Padding.y + TextHeight, 1.0f, 10.0f),
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								EG::RGBA16_LightGrey()
							);
			}

		}

		void Init()
		{}

		ButtonState State;
		HoveredState HoverState;
		std::string Text;
		EG::ColorRGBA16 TextColor;
		EG::Fonts::Font* TextFont;
		float FontScale;
		float caret_count;
		int32_t CursorIndex;
		int32_t caret_on;
		int32_t KeyboardInFocus;

	
		EGUI::Signal<float> on_click;	
		EGUI::Signal<std::string> on_keyboard;
		EGUI::Signal<> on_backspace;
		EGUI::Signal<> on_enter;
	};

	// Group is responsible for holding other gui elements and then transforming them together
	struct GUIGroup : GUIElement<GUIGroup>
	{
		GUIGroup() 
		{ 
			// Init
			Init();
		}

		GUIGroup(EM::Vec2 P)
		{
			Position = P;

			// Init
			Init();
		}
	
		void Init()
		{
			// Set up type
			Type = GUIType::GROUP; 

			// Set up member variables
			ElementIndex 	= 2;
			X0Offset 		= 15.0f;
			X1Offset		= 140.0f;
			YOffset 		= 25.0f;						// Not exact way but close estimate for now
			TitlePadding 	= 15.0f;
			Name 			= std::string("GUIGroup");		// Default Name
			Dimensions		= EM::Vec2(250.0f, 300.0f);		// Default Dimensions
			TextColor		= EG::RGBA16_MidGrey();
			Color 			= EG::RGBA16(0.12, 0.12, 0.12, 1.0f);
			TextFont 		= nullptr;

			// Get font
			FontScale = 1.0f;

			// Set up GUIGroup's on_hover signal
			on_hover.connect([&]()
			{
				HoverState = HoveredState::ON_HOVER;
			});

			// Set up GUIGroup's off_hover signal
			off_hover.connect([&]()
			{
				HoverState = HoveredState::OFF_HOVER;
			});
		}

		void AddToGroup(GUIElementBase* Element, const std::string& Name)
		{
			// Push back into group's children
			Children.push_back(Element);

			// Set Group as parent of child
			Element->Parent = this;

			// Set up position of Element in relation to group
			Element->Position = EM::Vec2(Position.x + Dimensions.x - X1Offset, Position.y + Dimensions.y - ElementIndex * YOffset - TitlePadding);
			Element->Name = Name;

			// Increment element index
			ElementIndex++;	
		}

		void Update()
		{
			// Loop through all children and update their positions based on their index in the vector
			auto index = 2;
			for(auto C : Children)
			{
				// Update position
				C->Position = EM::Vec2(Position.x + Dimensions.x - X1Offset, Position.y + Dimensions.y - index * YOffset - TitlePadding);

				// Update AABB
				C->AABB.Min = C->Position;
				C->AABB.Max = C->AABB.Min + C->Dimensions;

				// Call update on child
				C->Update();
				index++;
			}
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			// Draw Group border
			Batch->Add(	
						EM::Vec4(Position, Dimensions),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						Color,
						0.0f,
						EG::SpriteBatch::DrawOptions::BORDER | EG::SpriteBatch::DrawOptions::SHADOW,
						EG::SetOpacity(EG::RGBA16_DarkGrey(), 0.4f),
						1.0f
					);

			if (TextFont == nullptr) TextFont = EG::FontManager::GetFont("WeblySleek_12");

			// Draw Title border
			Batch->Add(	
						EM::Vec4(Position.x, Position.y + Dimensions.y - YOffset, Dimensions.x, YOffset),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						Color,
						0.0f,
						EG::SpriteBatch::DrawOptions::BORDER,
						EG::SetOpacity(EG::RGBA16_DarkGrey(), 0.8f),
						1.0f
					);

			// Draw title of widget
			auto TitleFont = EG::FontManager::GetFont("WeblySleek");
			// Calculate total width of title to find placement
			float TitleAdvance = 0.0f;
			for (auto& c : Name)
			{
				TitleAdvance += EG::Fonts::GetAdvance(c, TitleFont, 1.0f);
			}

			EG::Fonts::PrintText(
									Position.x + Dimensions.x / 2.0f - TitleAdvance / 2.0f,
									Position.y + Dimensions.y - YOffset + 5.0f,
									1.0f,
									Name,
									TitleFont,
									*Batch,
									EG::RGBA16_MidGrey()
								);


			// Try and draw this shiz
			for(auto& E : Children)
			{
				// Print name of child
				EG::Fonts::PrintText(
										Position.x + X0Offset, 								// X Position
										E->Position.y + 5.0f,										// Y Position
										FontScale,											// Font Scale
										E->Name + std::string(":"),									// Child Name
										TextFont,												// Font
										*Batch,														// SpriteBatch
										TextColor												// Font Color
									);

				// Print Child contents
				E->Draw(Batch);
			}

		}

		// Vector of children
		std::vector<GUIElementBase*> Children;
		EG::Fonts::Font* TextFont;
		EG::ColorRGBA16 TextColor;
		float TitlePadding;
		float FontScale;
		float X0Offset;
		float X1Offset;
		float YOffset;
		int32_t ElementIndex;
	};

}}


#endif