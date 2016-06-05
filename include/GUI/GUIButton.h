#ifndef GUIBUTTON_H
#define GUIBUTTON_H

#include <iostream>

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

		std::vector<ImageFrame> Frames;   // Could totally put this in a resource manager of some sort
	};

	inline void DoSomething(GUIButton* Button)
	{
		std::cout << "Hello!" << std::endl;
	}
}}


#endif