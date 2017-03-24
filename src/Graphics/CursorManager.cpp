#include "Graphics/CursorManager.h"


namespace Enjon { namespace CursorManager {

	std::unordered_map<std::string, SDL_Cursor*> Cursors;

	void Init()
	{
		Cursors["Arrow"] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
		Cursors["IBeam"] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	}

	// Get specificed cursor with given name
	SDL_Cursor* Get(const std::string& S)
	{
		auto search = Cursors.find(S);
		if (search != Cursors.end())
		{
			return search->second;
		}

		return Cursors["Arrow"];
	}

	// Delete all cursors and clear any memory used
	void DeleteCursors()
	{
		// Free memory eventually and things...
	}

}}

