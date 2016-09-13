#ifndef ENJON_WINDOW_H
#define ENJON_WINDOW_H 

#include <string>

#include <SDL2/SDL.h>
#include <GLEW/glew.h>

#include "Graphics/Vertex.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Graphics {


	enum WindowFlags { DEFAULT = 0x0, INVISIBLE = 0x1, FULLSCREEN = 0x2, BORDERLESS = 0x4 };
	enum MouseCursorFlags{ GET_STATUS = -1, HIDE = 0, SHOW = 1 };


	class Window
	{
	public:
		Window();
		~Window();

		int Init(std::string windowName, int screenWidth, int screenHeight, WindowFlags currentFlags = WindowFlags::DEFAULT); 
		void SetWindowTitle(const char* title);
		void SetWindowFullScreen(int screenWidth, int screenHeight);

		bool inline IsFullScreen() const { return m_isfullscreen; }

		inline int ShowMouseCursor(int val) const { return SDL_ShowCursor(val); } 
		inline int GetScreenWidth() const { return m_screenWidth; }
		inline int GetScreenHeight() const { return m_screenHeight; }
	
		void Clear(float val, GLbitfield mask, const ColorRGBA16& color = RGBA16_DarkGrey());
		void SwapBuffer(); 

		SDL_Window* GetWindowContext() { return m_sdlWindow; }

	private:
		SDL_Window* m_sdlWindow;
		int m_screenWidth;
		int m_screenHeight;
		bool m_isfullscreen;
	};

}}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif