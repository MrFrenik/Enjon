#ifndef GUIBUTTON_H
#define GUIBUTTON_H

#include <iostream>

#include "Defines.h"
#include "Graphics/Color.h"
#include "GUI/GUIElement.h"
#include "Graphics/Animations.h"

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

	inline void DoSomething(GUIButton* Button)
	{
		std::cout << "Hello!" << std::endl;
	}

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

		std::vector<GUITextButton*> List;
	};


	// GUI Radial Button
	struct GUIRadialButton : GUIElement<GUIRadialButton>
	{
		GUIRadialButton() 
		{ 
			Type = GUIType::RADIAL_BUTTON; 
	
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

			// Load in color themes from data
			this->ActiveColor 	= EG::RGBA16_LimeGreen();
			this->InactiveColor = EG::RGBA16_DarkGrey();

			// Set up initial color
			this->Color = this->InactiveColor;

			// Set up states
			this->State 		= ButtonState::INACTIVE;
			this->HoverState 	= HoveredState::OFF_HOVER;
		}

		void Init() {}

		EG::ColorRGBA16 ActiveColor;
		EG::ColorRGBA16 InactiveColor;
	};

}}


#endif