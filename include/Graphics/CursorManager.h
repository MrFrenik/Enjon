#ifndef CURSOR_MANAGER_H
#define CURSOR_MANAGER_H

#include <SDL2/SDL.h>
#include "Utils/Errors.h"

#include <unordered_map>

namespace Enjon { namespace Graphics { namespace CursorManager {

	/* Init the CursorManager */
	void Init();

	// Get specificed cursor with given name
	SDL_Cursor* Get(const std::string& S);

	// Delete all cursors and clear any memory used
	void DeleteCursors();

}}}



#endif