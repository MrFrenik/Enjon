#ifndef GUIBUTTON_H
#define GUIBUTTON_H

#include <iostream>

#include "Defines.h"
#include "Graphics/Color.h"
#include "GUI/GUIElement.h"
#include "Graphics/Animations.h"
#include "Math/Vec2.h"
#include "Math/Maths.h"

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
			Dimensions 			= EM::Vec2(130.0f, 20.0f);

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

		void Update()
		{
			
		}


		void Draw(EG::SpriteBatch* TB)
		{
			
		}

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
			this->TextColor 	= EG::RGBA16_White();
			this->Name 			= std::string("GUITextButton");
			this->TextPadding	= EM::Vec2(5.0f, 5.0f);

			// Dimensions
			Dimensions 			= EM::Vec2(160.0f, 18.0f);

			// Get font
			this->TextFont 		= nullptr;
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

		void Update()
		{
			
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			// If null
			if (this->TextFont == nullptr) this->TextFont = EG::FontManager::GetFont("WeblySleek_10");

			// Draw box
			Batch->Add(
						EM::Vec4(
									this->AABB.Min,
									this->AABB.Max - this->AABB.Min
								),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id, 
						this->Color,
						this->Depth,
						EG::SpriteBatch::DrawOptions::BORDER,
						EG::SetOpacity(EG::RGBA16(0.08f, 0.08f, 0.08f, 1.0f), 1.0f)
					);

			// Draw text
			EG::Fonts::PrintText(	
									Position.x + TextPadding.x, 
									Position.y + TextPadding.y, 
									this->FontScale, 
									this->Text, 
									this->TextFont, 
									*Batch, 
									this->TextColor,
									EG::Fonts::TextStyle::SHADOW,
									0.0f, 
									2.0f
								);
		}

		EM::Vec2 TextPadding;
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
			this->Dimensions = EM::Vec2(145.0f, 18.0f);

			ValueUp.Dimensions 		= EM::Vec2(20.0f, 18.0f);
			ValueDown.Dimensions 	= EM::Vec2(20.0f, 18.0f);
			ValueText.Dimensions 	= EM::Vec2(85.0f, 18.0f);

			ValueText.Text = "0.50";

			// These aren't working...
			ValueUp.Parent 		= this;
			ValueDown.Parent 	= this;
			ValueUp.Parent 		= this;

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

		void Draw(EG::SpriteBatch* Batch)
		{
			// Draw Value Up
			Batch->Add(
						EM::Vec4(ValueText.AABB.Min, ValueText.AABB.Max - ValueText.AABB.Min),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						ValueText.Color, 
						0.0f, 
						EG::SpriteBatch::DrawOptions::BORDER, 
						EG::SetOpacity(EG::RGBA16(0.18f, 0.18f, 0.18f, 1.0f), 0.5f)					
					);
			
			// Draw Text Box
			ValueText.Draw(Batch);

			// Draw Value Up
			Batch->Add(
						EM::Vec4(ValueUp.AABB.Min, ValueUp.AABB.Max - ValueUp.AABB.Min),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						ValueUp.Color,
						0.0f, 
						EG::SpriteBatch::DrawOptions::BORDER, 
						EG::RGBA16_Black()
					);


			auto F = EG::FontManager::GetFont("WeblySleek");
			EG::Fonts::PrintText(	
									ValueUp.Position.x + ValueUp.Dimensions.x / 2.0f - EG::Fonts::GetAdvance('+', F, 1.0f) / 2.0f, 
									ValueUp.Position.y + ValueUp.Dimensions.y / 2.0f - EG::Fonts::GetHeight('-', F, 1.0f) / 2.0f, 
									1.0f, 
									"+", 
									EG::FontManager::GetFont("WeblySleek"), 
									*Batch, 
									ValueText.TextColor	
								);

			// Draw Value Down
			Batch->Add(
						EM::Vec4(ValueDown.AABB.Min, ValueDown.AABB.Max - ValueDown.AABB.Min),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						ValueDown.Color,
						0.0f, 
						EG::SpriteBatch::DrawOptions::BORDER, 
						EG::RGBA16_Black()
					);

			EG::Fonts::PrintText(	
									ValueDown.Position.x + ValueDown.Dimensions.x / 2.0f - EG::Fonts::GetAdvance('-', F, 1.0f) / 2.0f, 
									ValueDown.Position.y + ValueDown.Dimensions.y / 2.0f - EG::Fonts::GetHeight('-', F, 1.0f) / 2.0f, 
									1.0f, 
									"-", 
									EG::FontManager::GetFont("WeblySleek"), 
									*Batch, 
									ValueText.TextColor	
								);
		}

		void Update()
		{
			// Update positions and AABB of Value Text
			ValueText.Position = Position;
			ValueText.AABB.Min = ValueText.Position;
			ValueText.AABB.Max = ValueText.AABB.Min + ValueText.Dimensions;	

			ValueUp.Position = EM::Vec2(Position.x + ValueText.Dimensions.x, Position.y);
			ValueUp.AABB.Min = ValueUp.Position;
			ValueUp.AABB.Max = ValueUp.AABB.Min + ValueUp.Dimensions;	

			ValueDown.Position = EM::Vec2(ValueUp.Position.x + ValueUp.Dimensions.x, Position.y);
			ValueDown.AABB.Min = ValueDown.Position;
			ValueDown.AABB.Max = ValueDown.AABB.Min + ValueDown.Dimensions;	
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
			this->TextFont 		= nullptr;
			this->FontScale = 1.0f;
			this->Name = std::string("GUIDropDownButton");
			this->TextColor		= EG::RGBA16_White();
			this->TextPadding = EM::Vec2(5.0f, 6.0f);
			this->BorderColor = EG::SetOpacity(EG::RGBA16(0.18f, 0.18f, 0.18f, 1.0f), 0.5f);
			this->YOffset = 20.0f;
			this->XPadding = 1.0f;

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
					this->BorderColor = EG::SetOpacity(EG::RGBA16(0.20f, 0.635f, 1.0f, 1.0f), 0.5f);
					this->State = ButtonState::ACTIVE;
				} 
				else
				{
					this->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
					this->BorderColor = EG::SetOpacity(EG::RGBA16(0.18f, 0.18f, 0.18f, 1.0f), 0.5f);
					this->State = ButtonState::INACTIVE;
				} 
			});
		}

		void Init()
		{}

		void Update() 
		{
			// Loop through all children and update their AABB's and positions
			auto index = 1;
			for (auto C : List)
			{
				// Update AABB of child
				C->Position = EM::Vec2(Position.x + XPadding, Position.y - YOffset * index);
				C->AABB.Min = C->Position;
				C->AABB.Max = C->AABB.Min + C->Dimensions;
				index++;
			}
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			if (this->TextFont == nullptr) this->TextFont = EG::FontManager::GetFont("WeblySleek_10");

			Batch->Add(
							EM::Vec4(Position.x, Position.y, this->Dimensions),
							EM::Vec4(0, 0, 1, 1),
							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
							Color, 
							0.0f, 
							(EG::SpriteBatch::DrawOptions::BORDER), 
							this->BorderColor	
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

			EG::Fonts::PrintText(	
									Position.x + TextPadding.x + 112.0f, 
									Position.y + TextPadding.y + 4.0f, 
									this->FontScale * 0.8f, 
									"d", 
									EG::FontManager::GetFont("Arrows7"), 
									*Batch, 
									this->TextColor, 
									EG::Fonts::TextStyle::SHADOW,
									EM::ToRadians(180.0f)
								);

			// If activated
			if (this->State)
			{
				float XPadding = 5.0f;
				auto Amount = this->List.size();
				auto GroupWidth = 160.0f;

				// Draw box for group
				Batch->Add(
							EM::Vec4(
										this->Position.x,
										this->Position.y - this->YOffset * Amount - 2.0f, 
										GroupWidth, 
										20.0f * Amount
									),
							EM::Vec4(0, 0, 1, 1),
							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id, 
							EG::RGBA16_DarkGrey(), 
							0.0f,
							EG::SpriteBatch::DrawOptions::BORDER | EG::SpriteBatch::DrawOptions::SHADOW,
							EG::RGBA16_DarkGrey(), 
							1.0f
						);

				// Draw each child
				for(auto C : this->List)
				{
					C->Depth = 1.0f;
					C->Draw(Batch);
				}

			}	
		}

		void CalculateDimensions()
		{
			std::string& T = this->Text;
			float A = 0.0f;
			auto period_count = 0;
			auto index = 0;

			std::cout << T << std::endl;

			if (this->TextFont == nullptr) this->TextFont = EG::FontManager::GetFont("WeblySleek_10");

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
				} 

				index++;
			}

			this->Dimensions = EM::Vec2(A + 20.0f, 20.0f);
		}

		std::vector<GUITextButton*> List;
		EG::Fonts::Font* TextFont;
		EG::ColorRGBA16 TextColor;
		EG::ColorRGBA16 BorderColor;
		EM::Vec2 TextPadding;
		float XPadding;
		float FontScale;
		float YOffset;
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

		void Update()
		{
			
		}

		void Draw(EG::SpriteBatch* TB)
		{
			
		}

		EG::ColorRGBA16 ActiveColor;
		EG::ColorRGBA16 InactiveColor;
	};

}}


#endif