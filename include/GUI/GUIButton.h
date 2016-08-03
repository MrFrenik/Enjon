#ifndef GUIBUTTON_H
#define GUIBUTTON_H

#include <iostream>

#include "Defines.h"
#include "Graphics/Color.h"
#include "GUI/GUIElement.h"
#include "Graphics/Animations.h"
#include "Math/Vec2.h"

namespace Enjon { namespace GUI {

	// Button
	struct GUIButton : GUIElement<GUIButton>
	{
		GUIButton() 
		{ 
			Type = GUIType::BUTTON; 
		
			// Set up initial states	
			this->State 		= ButtonState::INACTIVE;
			this->HoverState 	= HoveredState::OFF_HOVER;
			this->Color 		= EG::RGBA16_LightGrey();
			this->Name 			= std::string("GUIButton");

			// Set up PlayButton's on_hover signal
			this->on_hover.connect([&]()
			{
				// We'll just change a color for now
				this->Color = EG::RGBA16_White();

				// Set state to active
				this->HoverState = HoveredState::ON_HOVER;
			});

			// Set up PlayButton's off_hover signal
			this->off_hover.connect([&]()
			{
				this->Color = EG::RGBA16_LightGrey();

				// Set state to inactive
				this->HoverState = HoveredState::OFF_HOVER;
			});
		}

		void Init()
		{}

		std::vector<EA::ImageFrame> Frames;   // Could totally put this in a resource manager of some sort
	};

	// Button with Text
	struct GUITextButton : GUIElement<GUITextButton>
	{
		GUITextButton() 
		{ 
			this->Type = GUIType::TEXT_BUTTON; 

			// Set up states
			this->State = ButtonState::INACTIVE;
			this->HoverState = HoveredState::OFF_HOVER;

			// Set up color
			this->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);

			// Set up text color
			this->TextColor = EG::RGBA16_MidGrey();
			this->Name 			= std::string("GUITextButton");

			// Get font
			this->FontScale		= 1.0f;

			// Set up off_hover signal
			this->on_hover.connect([&]()
			{
				this->HoverState = HoveredState::ON_HOVER;
				this->Color = EG::RGBA16(0.1f, 0.1f, 0.1f, 1.0f);
				this->TextColor = EG::RGBA16_White();
			});

			// Set up on_hover signal
			this->off_hover.connect([&]()
			{
				this->HoverState = HoveredState::OFF_HOVER;
				this->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
				this->TextColor = EG::RGBA16_MidGrey();
			});
		}

		void Init()
		{}

		EGUI::Signal<GUIElementBase*> on_click;
		EGUI::Signal<> on_hover;
		EGUI::Signal<> off_hover;
		EG::ColorRGBA16 TextColor;
		EG::Fonts::Font* TextFont;
		float FontScale;
	};

	// Value Text Box Button (Acts as a button group, which maybe would be better)
	struct GUIValueButton : GUIElement<GUIValueButton>
	{
		GUIValueButton() 
		{ 
			// Set up type
			this->Type = GUIType::VALUE_BUTTON; 

			// Member variables
			this->State = ButtonState::INACTIVE;
			this->HoverState = HoveredState::OFF_HOVER;

			this->Value = 0.0f;
			this->Step = 0.1f;

			// Set up connect for Value Up
			this->ValueUp.on_click.connect([&]()
			{
				// Value goes up
				Value += Step;
			});

			// Set up connect for Value Down
			this->ValueDown.on_click.connect([&]()
			{
				// Value goes up
				Value -= Step;
			});

			this->ValueText.on_enter.connect([&]()
			{
				// Set value to text
				Value = std::atof(ValueText.Text.c_str());
			});
		}

		void Init() {}

		float Value;
		float Step;
		GUIButton ValueUp;
		GUIButton ValueDown;
		GUITextBox ValueText;
	};

	// DropDownMenuButton
	struct GUIDropDownButton : GUIElement<GUIDropDownButton>
	{
		GUIDropDownButton() 
		{ 
			// Set up type
			this->Type = GUIType::DROP_DOWN_BUTTON; 

			// Member variables
			this->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
			this->State = ButtonState::INACTIVE;
			this->HoverState = HoveredState::OFF_HOVER;

			// Get font
			if (!EG::FontManager::IsInit()) EG::FontManager::Init();
			this->TextFont = EG::FontManager::GetFont("WeblySleek_10");
			this->FontScale = 1.0f;
			this->Name = std::string("GUIDropDownButton");
			this->TextColor		= EG::RGBA16_MidGrey();
			this->TextPadding = EM::Vec2(5.0f, 6.0f);

			// Set up Drop Down Menu Button's on_hover signal
			this->on_hover.connect([&]()
			{
				this->HoverState = HoveredState::ON_HOVER;
				this->Color = EG::RGBA16(0.1f, 0.1f, 0.1f, 1.0f);
			});

			// Set up Drop Down Menu Button's off_hover signal
			this->off_hover.connect([&]()
			{
				this->HoverState = HoveredState::OFF_HOVER;
				if (!this->State) this->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
			});

			// Set up Drop Down Menu Button's on_click signal
			this->on_click.connect([&]()
			{
				// Need a drop down box here with all the options to be selected
				if (this->State == ButtonState::INACTIVE)
				{
					this->Color = EG::RGBA16(0.08f, 0.08f, 0.08f, 1.0f);
					this->State = ButtonState::ACTIVE;
				} 
				else
				{
					this->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
					this->State = ButtonState::INACTIVE;
				} 
			});
		}

		void Init()
		{}

		void Draw(EG::SpriteBatch* Batch)
		{
			Batch->Add(
							EM::Vec4(Position.x, Position.y, 100.0f, 20.0f),
							EM::Vec4(0, 0, 1, 1),
							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
							Color	
						);

			EG::Fonts::PrintText(	
									Position.x + TextPadding.x, 
									Position.y + TextPadding.y, 
									this->FontScale, 
									this->Text, 
									this->TextFont, 
									*Batch, 
									this->TextColor	
								);
		}

		void CalculateDimensions()
		{
			std::string& T = this->Text;
			float A = 0.0f;
			auto period_count = 0;
			auto index = 0;

			std::cout << T << std::endl;

			// Get advance
			for (auto& c : T)
			{
				// Summation of all characters
				A += EG::Fonts::GetAdvance(c, this->TextFont, this->FontScale);

				if (A > 90.0f) 
				{
					c = '.';
					period_count++;
				}

				if (period_count >= 4)
				{
					// Get substring and replace it
					T = T.substr(0, index);
					break;			
			if (!EG::FontManager::IsInit()) EG::FontManager::Init(); 		// Make sure FontManager is initialized
			this->TextFont = EG::FontManager::GetFont("WeblySleek_10");
				} 

				index++;
			}

			this->Dimensions = EM::Vec2(A + 20.0f, 20.0f);
		}

		std::vector<GUITextButton*> List;
		EG::Fonts::Font* TextFont;
		EG::ColorRGBA16 TextColor;
		EM::Vec2 TextPadding;
		float FontScale;
	};


	// GUI Radial Button
	struct GUIRadialButton : GUIElement<GUIRadialButton>
	{
		GUIRadialButton() 
		{ 
			Type = GUIType::RADIAL_BUTTON; 

			// Load in color themes from data
			this->ActiveColor 	= EG::RGBA16_LimeGreen();
			this->InactiveColor = EG::RGBA16_DarkGrey();

			// Set up initial color
			this->Color = this->InactiveColor;
			this->Name 			= std::string("GUIRadialButton");

			// Set up states
			this->State 		= ButtonState::INACTIVE;
			this->HoverState 	= HoveredState::OFF_HOVER;
	
			// Set up ToggleOnionSkin's on_hover signal
			this->on_hover.connect([&]()
			{
				this->HoverState = HoveredState::ON_HOVER;
			});

			// Set up ToggleOnionSkin's off_hover signal
			this->off_hover.connect([&]()
			{
				this->HoverState = HoveredState::OFF_HOVER;
			});

			this->on_click.connect([&]()
			{
				// Toggle on and off
				this->State = this->State == ButtonState::INACTIVE ? ButtonState::ACTIVE : ButtonState::INACTIVE;

				// Toggle color
				this->Color = this->State == ButtonState::INACTIVE ? this->InactiveColor : this->ActiveColor;
			});

		}

		void Init() {}

		EG::ColorRGBA16 ActiveColor;
		EG::ColorRGBA16 InactiveColor;
	};

}}


#endif