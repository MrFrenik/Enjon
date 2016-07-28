/*-- Enjon includes --*/
#include "Editor/AnimationEditor.h"

/*-- IsoARPG includes --*/
#include "EnjonAnimation.h"
#include "AnimationManager.h"
#include "AnimManager.h"
#include "SpatialHash.h"
#include "Level.h"

/*-- Standard Library includes --*/
#include <unordered_map>

/*-- 3rd Party Includes --*/
#include <SDL2/SDL.h>

using namespace EA;
using json = nlohmann::json;

namespace Enjon { namespace GUI {

	// Something like this eventually for global gui references...
	namespace ButtonManager
	{
		std::unordered_map<std::string, GUIButton*> Buttons;

		void Add(std::string S, GUIButton* B)
		{
			Buttons[S] = B;
		}

		GUIButton* Get(const std::string S)
		{
			auto search = Buttons.find(S);
			if (search != Buttons.end())
			{
				return search->second;
			}	

			return nullptr;
		}
	};

	// This is stupid, but it's for testing...
	namespace TextBoxManager
	{
		std::unordered_map<std::string, GUITextBox*> TextBoxes;

		void Add(std::string S, GUITextBox* T)
		{
			TextBoxes[S] = T;
		}

		GUITextBox* Get(const std::string S)
		{
			auto search = TextBoxes.find(S);
			if (search != TextBoxes.end())
			{
				return search->second;
			}
			return nullptr;
		}
	};

	namespace GUIManager
	{
		std::unordered_map<std::string, GUIElementBase*> Elements;

		void Add(std::string S, GUIElementBase* E)
		{
			Elements[S] = E;
		}

		GUIElementBase* Get(const std::string S)
		{
			auto search = Elements.find(S);
			if (search != Elements.end())
			{
				return search->second;
			}

			return nullptr;
		}
	}
}}

namespace CameraManager
{
	std::unordered_map<std::string, EG::Camera2D*> Cameras;

	void AddCamera(std::string S, EG::Camera2D* C)
	{
		Cameras[S] = C;
	}

	EG::Camera2D* GetCamera(const std::string S)
	{
		auto search = Cameras.find(S);
		if (search != Cameras.end())
		{
			return search->second;
		}
		return nullptr;
	}
};

namespace CursorManager
{
	std::unordered_map<std::string, SDL_Cursor*> Cursors;

	void Init()
	{
		Cursors["Arrow"] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
		Cursors["IBeam"] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	}

	SDL_Cursor* Get(const std::string S)
	{
		auto search = Cursors.find(S);
		if (search != Cursors.end())
		{
			return search->second;
		}
		return nullptr;
	}

};

namespace Enjon { namespace AnimationEditor {

	bool Init()
	{
		return true;	
	}

	bool Update()
	{
		return true;
	}		

	bool Draw()
	{
		return true;
	}

}}
