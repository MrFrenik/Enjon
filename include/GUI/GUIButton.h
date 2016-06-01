#ifndef GUIBUTTON_H
#define GUIBUTTON_H

#include <iostream>

#include "GUI/GUIElement.h"

namespace Enjon { namespace GUI {

	inline void DoSomething(GUIButton* Button)
	{
		std::cout << "Hello!" << std::endl;
	}
}}


#endif