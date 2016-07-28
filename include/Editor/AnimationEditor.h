#ifndef ANIMATION_EDITOR_H
#define ANIMATION_EDITOR_H

/*-- Enjon includes --*/
#include "GUI/GUIButton.h"
#include "GUI/GUIElement.h"
#include "Graphics/Camera2D.h"

/*-- Standard Library includes --*/
#include <unordered_map>

/*-- 3rd Party Includes --*/
#include <SDL2/SDL.h>


namespace Enjon { namespace GUI {

	// Something like this eventually for global gui references...
	namespace ButtonManager
	{
		void Add(std::string S, GUIButton* B);

		GUIButton* Get(const std::string S);
	};

	// This is stupid, but it's for testing...
	namespace TextBoxManager
	{
		void Add(std::string S, GUITextBox* T);

		GUITextBox* Get(const std::string S);
	};

	namespace GUIManager
	{
		void Add(std::string S, GUIElementBase* E);

		GUIElementBase* Get(const std::string S);
	}
}}

namespace CameraManager
{
	void AddCamera(std::string S, EG::Camera2D* C);

	EG::Camera2D* GetCamera(const std::string S);
};

namespace CursorManager
{
	void Init();

	SDL_Cursor* Get(const std::string S);
};

namespace Enjon { namespace AnimationEditor {

	bool Init();

	bool Update();		

	bool Draw();
}}


#endif