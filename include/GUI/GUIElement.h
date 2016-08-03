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
		virtual void Draw(EG::SpriteBatch* Batch) = 0;

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
			this->Name 			= std::string("GUITextBox");
			this->Text 			= std::string("");
			this->CursorIndex 	= 0;
			this->TextColor 	= EG::RGBA16_White();
			this->Color 	 	= EG::RGBA16_DarkGrey();
			this->caret_on 		= false;
			this->caret_count 	= 0.0f;

			// Initial states
			this->State 		= ButtonState::INACTIVE;
			this->HoverState 	= HoveredState::OFF_HOVER;

			// Get font
			this->FontScale = 1.0f;

			// Set up TextBox's on_hover signal
			this->on_hover.connect([&]()
			{
				// Change the mouse cursor
				SDL_SetCursor(EG::CursorManager::Get("IBeam"));

				this->HoverState = HoveredState::ON_HOVER;

				// Change color of Box
				this->Color = EG::SetOpacity(EG::RGBA16_LightGrey(), 0.3f);

			});

			// Set up TextBox's off_hover signal
			this->off_hover.connect([&]()
			{
				// Change mouse cursor back to defaul
				SDL_SetCursor(EG::CursorManager::Get("Arrow"));

				this->HoverState = HoveredState::OFF_HOVER;
			
				// Change color of Box
				this->Color = EG::RGBA16_DarkGrey();
			});

			// Set up TextBox's on_keyboard signal
			this->on_keyboard.connect([&](std::string c)
			{
				auto str_len = this->Text.length();
				auto cursor_index = this->CursorIndex;

				// std::cout << cursor_index << std::endl;

				// End of string
				if (cursor_index >= str_len)
				{
					this->Text += c;
					this->CursorIndex = str_len + 1;
				}
				// Cursor somewhere in the middle of the string
				else if (cursor_index > 0)
				{
					auto FirstHalf = this->Text.substr(0, cursor_index);
					auto SecondHalf = this->Text.substr(cursor_index, str_len);

					FirstHalf += c; 
					this->Text = FirstHalf + SecondHalf;
					this->CursorIndex++;
				}
				// Beginning of string
				else
				{
					this->Text = c + this->Text;
					this->CursorIndex++;
				}
			});

			// Set up TextBox's on_backspace signal
			this->on_backspace.connect([&]()
			{
				auto str_len = this->Text.length();
				auto cursor_index = this->CursorIndex;

				// erase from string
				if (str_len > 0 && cursor_index > 0)
				{
					auto S1 = this->Text.substr(0, cursor_index - 1);
					std::string S2;

					if (cursor_index + 1 < str_len) S2 = this->Text.substr(cursor_index, str_len);

					S1.erase(cursor_index - 1);
					this->Text = S1 + S2;
					this->CursorIndex--;
				}
			});

			this->on_click.connect([&](EM::Vec2& MouseCoords)
			{
				std::string& Text = this->Text;
				auto XAdvance = this->Position.x;
				uint32_t index = 0;

				// Get advance
				for (auto& c : Text)
				{
					float Advance = EG::Fonts::GetAdvance(c, this->TextFont, this->FontScale);
					if (XAdvance + Advance < MouseCoords.x) 
					{
						XAdvance += Advance;
						index++;
					}
					else break;
				}

				this->CursorIndex = index;

				// set caret on to true and count to 0
				this->caret_count = 0.0f;
				this->caret_on = true;
			});
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

	
		EGUI::Signal<EM::Vec2&> on_click;	
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
			this->Init();
		}

		GUIGroup(EM::Vec2 P)
		{
			this->Position = P;

			// Init
			this->Init();
		}
	
		void Init()
		{
			// Set up type
			this->Type = GUIType::GROUP; 

			// Set up member variables
			this->ElementIndex 	= 1;
			this->X0Offset 		= 10.0f;
			this->X1Offset		= 100.0f;
			this->YOffset 		= 20.0f;						// Not exact way but close estimate for now
			this->Name 			= std::string("GUIGroup");		// Default Name
			this->Dimensions	= EM::Vec2(250.0f, 300.0f);		// Default Dimensions
			this->TextColor		= EG::RGBA16_MidGrey();
			this->Color 		= EG::RGBA16(0.12, 0.12, 0.12, 1.0f);

			// Get font
			if (!EG::FontManager::IsInit()) EG::FontManager::Init();
			this->TextFont = EG::FontManager::GetFont("WeblySleek_12");
			this->FontScale = 1.0f;

			// Set up GUIGroup's on_hover signal
			this->on_hover.connect([&]()
			{
				this->HoverState = HoveredState::ON_HOVER;
			});

			// Set up GUIGroup's off_hover signal
			this->off_hover.connect([&]()
			{
				this->HoverState = HoveredState::OFF_HOVER;
			});
		}

		void AddToGroup(GUIElementBase* Element, const std::string& Name)
		{
			// Push back into group's children
			this->Children.push_back(Element);

			// Set Group as parent of child
			Element->Parent = this;

			// Set up position of Element in relation to group
			Element->Position = EM::Vec2(Position.x + this->X1Offset, Position.y + Dimensions.y - this->ElementIndex * this->YOffset);
			Element->Name = Name;

			// Increment element index
			this->ElementIndex++;	
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			// Draw Group border
			Batch->Add(	
						EM::Vec4(this->Position, this->Dimensions),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						this->Color
					);

			auto F = EG::FontManager::GetFont("WeblySleek_12");

			// Try and draw this shiz
			for(auto& E : Children)
			{
				// Print name of child
				EG::Fonts::PrintText(
										this->Position.x + X0Offset, 								// X Position
										E->Position.y + 5.0f,										// Y Position
										this->FontScale,											// Font Scale
										E->Name + std::string(":"),									// Child Name
										this->TextFont,												// Font
										*Batch,														// SpriteBatch
										this->TextColor												// Font Color
									);

				// Print Child contents
				E->Draw(Batch);
			}

		}

		// Vector of children
		std::vector<GUIElementBase*> Children;
		EG::Fonts::Font* TextFont;
		EG::ColorRGBA16 TextColor;
		float FontScale;
		float X0Offset;
		float X1Offset;
		float YOffset;
		int32_t ElementIndex;
	};

	inline GUIGroup* AddToGroup(GUIGroup* Group, GUIElementBase* Element)
	{
		// Push back into group's children
		Group->Children.push_back(Element);

		// Set Group as parent of child
		Element->Parent = Group;

		return Group;
	}


}}


#endif