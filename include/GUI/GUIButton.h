#ifndef GUIBUTTON_H
#define GUIBUTTON_H

#include <iostream>

#include "Defines.h"
#include "GUI/GUIElement.h"
#include "Graphics/Animations.h"

namespace Enjon { namespace GUI {

	// Button
	struct GUIButton : GUIElement<GUIButton>
	{
		void Init()
		{
			std::cout << "Initialized Button..." << std::endl;
		}

		std::vector<EA::ImageFrame> Frames;   // Could totally put this in a resource manager of some sort
	};

	inline void DoSomething(GUIButton* Button)
	{
		std::cout << "Hello!" << std::endl;
	}

	// TextBox
	struct GUITextButton : GUIElement<GUITextButton>
	{
		void Init()
		{}

		EGUI::Signal<GUIElementBase*> on_click;
		EGUI::Signal<GUIElementBase*> on_hover;
		EGUI::Signal<GUIElementBase*> off_hover;
		EM::Vec2 Dimensions;
		std::vector<GUIButton*> List;
	};

	// TextBox
	struct GUIDropDownButton : GUIElement<GUIDropDownButton>
	{
		void Init()
		{}

		EM::Vec2 Dimensions;
		std::vector<GUITextButton*> List;
	};
}}


#endif